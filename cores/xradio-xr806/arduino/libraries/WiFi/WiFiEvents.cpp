#include "WiFiPrivate.h"

// the station address last reported with GOT_IP in this association, or 0
static uint32_t reportedIp = 0;

void wifiEventSendArduino(EventId event) {
	EventInfo eventInfo;
	memset(&eventInfo, 0, sizeof(EventInfo));
	if (!pWiFi)
		return; // failsafe
	pWiFi->postEvent(event, eventInfo);
}

static void wifiEventDisconnected(uint8_t reason) {
	reportedIp = 0;
	EventInfo eventInfo;
	memset(&eventInfo, 0, sizeof(EventInfo));
	eventInfo.wifi_sta_disconnected.reason = reason;
	pWiFi->postEvent(ARDUINO_EVENT_WIFI_STA_DISCONNECTED, eventInfo);
}

static void wifiEventConnected() {
	EventInfo eventInfo;
	memset(&eventInfo, 0, sizeof(EventInfo));
	wlan_sta_ap_t ap;
	if (wlan_sta_ap_info(&ap) == 0) {
		size_t len = MIN(ap.ssid.ssid_len, sizeof(eventInfo.wifi_sta_connected.ssid) - 1);
		memcpy(eventInfo.wifi_sta_connected.ssid, ap.ssid.ssid, len);
		eventInfo.wifi_sta_connected.ssid_len = len;
		eventInfo.wifi_sta_connected.channel  = ap.channel;
		eventInfo.wifi_sta_connected.authmode = wpaFlagsToAuthMode(ap.wpa_flags);
		memcpy(eventInfo.wifi_sta_connected.bssid, ap.bssid, 6);
	}
	pWiFi->postEvent(ARDUINO_EVENT_WIFI_STA_CONNECTED, eventInfo);
}

static void wifiEventGotIP(struct netif *nif, EventId event) {
	EventInfo eventInfo;
	memset(&eventInfo, 0, sizeof(EventInfo));
	if (!nif)
		return;
	eventInfo.got_ip.if_index			  = 0;
	eventInfo.got_ip.ip_changed			  = true;
	eventInfo.got_ip.ip_info.ip.addr	  = netif_ip4_addr(nif)->addr;
	eventInfo.got_ip.ip_info.gw.addr	  = netif_ip4_gw(nif)->addr;
	eventInfo.got_ip.ip_info.netmask.addr = netif_ip4_netmask(nif)->addr;
	pWiFi->postEvent(event, eventInfo);
}

static void wifiEventNetworkUp() {
	struct netif *nif = staNetif();
	if (nif == NULL)
		return;
	// with a static address, the connect handler and the NETWORK_UP that follows both get here
	uint32_t ip = netif_ip4_addr(nif)->addr;
	if (ip == reportedIp)
		return;
	reportedIp = ip;
	xr806_wlan_apply_power_save(nif, pDATA->sleep);
	wifiEventGotIP(nif, ARDUINO_EVENT_WIFI_STA_GOT_IP);
}

// runs in the sys_ctrl task, after the handler of net_ctrl.c (attached first)
static void networkEventHandler(uint32_t event, uint32_t data, void *arg) {
	if (!pWiFi)
		return; // failsafe
	bool ap = pDATA->mode == WIFI_MODE_AP;
	switch (EVENT_SUBTYPE(event)) {
		case NET_CTRL_MSG_WLAN_CONNECTED:
			if (ap)
				break;
			wifiEventConnected();
			// with the DHCP lease still bound (or a static address), net_ctrl.c brings the
			// netif up again without a NET_CTRL_MSG_NETWORK_UP
			if (NET_IS_IP4_VALID(staNetif()))
				wifiEventNetworkUp();
			break;
		case NET_CTRL_MSG_WLAN_DISCONNECTED:
			if (!ap)
				wifiEventDisconnected(WIFI_REASON_ASSOC_LEAVE);
			break;
		case NET_CTRL_MSG_CONNECTION_LOSS:
			wifiEventDisconnected(WIFI_REASON_BEACON_TIMEOUT);
			break;
		case NET_CTRL_MSG_WLAN_CONNECT_FAILED:
			wifiEventDisconnected(WIFI_REASON_NO_AP_FOUND);
			break;
		case NET_CTRL_MSG_WLAN_4WAY_HANDSHAKE_FAILED:
			wifiEventDisconnected(WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT);
			break;
		case NET_CTRL_MSG_WLAN_SCAN_SUCCESS:
			wifiScanDone(true);
			break;
		case NET_CTRL_MSG_WLAN_SCAN_FAILED:
			wifiScanDone(false);
			break;
		case NET_CTRL_MSG_NETWORK_UP:
			if (!ap)
				wifiEventNetworkUp();
			break;
		case NET_CTRL_MSG_NETWORK_DOWN:
			if (ap)
				break;
			reportedIp = 0;
			wifiEventSendArduino(ARDUINO_EVENT_WIFI_STA_LOST_IP);
			break;
	}
}

bool registerWifiHandlers() {
	observer_base *observer =
		sys_callback_observer_create(CTRL_MSG_TYPE_NETWORK, NET_CTRL_MSG_ALL, networkEventHandler, NULL);
	return observer != NULL && sys_ctrl_attach(observer) == 0;
}
