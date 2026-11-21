/* Copyright (c) Kuba Szczodrzyński 2022-06-26. */

#include "WiFiPrivate.h"

typedef struct {
	EventId event;
	uint16_t reason; // wifi_err_reason_t
} WiFiEventMsg;

extern void wifiScanDone(WiFiClass *cls);

static QueueHandle_t wifiEventQueueHandle = NULL;
static TaskHandle_t wifiEventTaskHandle	  = NULL;

static void wifiEventHandler(WiFiEventMsg msg) {
	if (!pWiFi)
		return; // failsafe
	LT_DM(WIFI, "Event %u (reason %u)", msg.event, msg.reason);

	EventInfo eventInfo;
	memset(&eventInfo, 0, sizeof(EventInfo));

	switch (msg.event) {
		case ARDUINO_EVENT_WIFI_SCAN_DONE:
			wifiScanDone(pWiFi);
			eventInfo.wifi_scan_done.status = 0;
			if (pWiFi->scan)
				eventInfo.wifi_scan_done.number = pWiFi->scan->count;
			break;
		case ARDUINO_EVENT_WIFI_STA_CONNECTED: {
			String ssid	   = pWiFi->SSID();
			uint8_t *bssid = pWiFi->BSSID();
			size_t ssidLen = min(ssid.length(), sizeof(eventInfo.wifi_sta_connected.ssid) - 1);
			memcpy(eventInfo.wifi_sta_connected.ssid, ssid.c_str(), ssidLen);
			eventInfo.wifi_sta_connected.ssid_len = ssidLen;
			eventInfo.wifi_sta_connected.channel  = pWiFi->channel();
			eventInfo.wifi_sta_connected.authmode = pWiFi->getEncryption();
			if (bssid)
				memcpy(eventInfo.wifi_sta_connected.bssid, bssid, 6);
			if (pDATA->useStaticIp)
				pDATA->staStatus = WL_CONNECTED;
			break;
		}
		case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
			if (pDATA->staLost) {
				// ARMINO would keep scanning for the lost network; with a scan pending,
				// bk_wifi_scan_start() blocks its caller for up to 30 s
				pDATA->staLost = false;
				bk_wifi_sta_stop();
			}
			eventInfo.wifi_sta_disconnected.reason = msg.reason;
			switch (msg.reason) {
				case WIFI_REASON_NO_AP_FOUND:
					pDATA->staStatus = WL_NO_SSID_AVAIL;
					break;
				case WIFI_REASON_AUTH_FAIL:
					pDATA->staStatus = WL_CONNECT_FAILED;
					break;
				case WIFI_REASON_BEACON_TIMEOUT:
					pDATA->staStatus = WL_CONNECTION_LOST;
					break;
				default:
					pDATA->staStatus = WL_DISCONNECTED;
					break;
			}
			break;
		case ARDUINO_EVENT_WIFI_STA_GOT_IP:
			eventInfo.got_ip.if_index			  = 0;
			eventInfo.got_ip.ip_changed			  = true;
			eventInfo.got_ip.ip_info.ip.addr	  = pWiFi->localIP();
			eventInfo.got_ip.ip_info.netmask.addr = pWiFi->subnetMask();
			eventInfo.got_ip.ip_info.gw.addr	  = pWiFi->gatewayIP();
			pDATA->staStatus					  = WL_CONNECTED;
			break;
		default:
			break;
	}
	pWiFi->postEvent(msg.event, eventInfo);
}

static void wifiEventTask(void *arg) {
	WiFiEventMsg msg;
	for (;;) {
		if (xQueueReceive(wifiEventQueueHandle, &msg, portMAX_DELAY) == pdTRUE)
			wifiEventHandler(msg);
	}
}

static void wifiEventSend(EventId event, uint16_t reason) {
	WiFiEventMsg msg = {.event = event, .reason = reason};
	if (wifiEventQueueHandle && wifiEventTaskHandle) {
		xQueueSend(wifiEventQueueHandle, &msg, portMAX_DELAY);
	} else {
		wifiEventHandler(msg);
	}
}

void wifiEventSendArduino(EventId event) {
	wifiEventSend(event, 0);
}

static uint16_t reasonToArduino(int reason) {
	switch (reason) {
		case WIFI_REASON_BEACON_LOST:
			return WIFI_REASON_BEACON_TIMEOUT;
		case ARMINO_WIFI_REASON_NO_AP_FOUND:
			return WIFI_REASON_NO_AP_FOUND;
		case WIFI_REASON_WRONG_PASSWORD:
			return WIFI_REASON_AUTH_FAIL;
		default:
			// 802.11 reason codes are the same in both APIs
			if (reason > 0 && reason < 200)
				return reason;
			return WIFI_REASON_UNSPECIFIED;
	}
}

// called from the ARMINO event task
static bk_err_t wifiEventCallback(void *arg, event_module_t module, int event_id, void *event_data) {
	if (!pWiFi)
		return BK_OK;
	if (module == EVENT_MOD_WIFI) {
		switch (event_id) {
			case EVENT_WIFI_SCAN_DONE:
				wifiEventSend(ARDUINO_EVENT_WIFI_SCAN_DONE, 0);
				break;
			case EVENT_WIFI_STA_CONNECTED:
				pDATA->staLinked = true;
				wifiEventSend(ARDUINO_EVENT_WIFI_STA_CONNECTED, 0);
				break;
			case EVENT_WIFI_STA_DISCONNECTED: {
				armino_wifi_event_sta_disconnected_t *info = (armino_wifi_event_sta_disconnected_t *)event_data;
				// ARMINO's own handler (registered first) has just switched the station back to DHCP
				if (pDATA->useStaticIp)
					bk_netif_set_ip4_config(NETIF_IF_STA, &pDATA->staIp);
				if (info)
					LT_DM(
						WIFI,
						"STA disconnected, ARMINO reason %d, local %d",
						info->disconnect_reason,
						info->local_generated
					);
				// without an existing link, ARMINO also reports state changes that are no connection failures:
				// - after a scan, wpa_supplicant leaves the scan state with reason 0 (not local),
				//   which does not abort a running connection attempt
				// - stopping the STA interface reports a local 802.11 reason (e.g. 3, DEAUTH_LEAVING)
				// real failures use ARMINO's own reasons (>= 256: NO_AP_FOUND, WRONG_PASSWORD, ...)
				if (!pDATA->staLinked && info &&
					(info->local_generated ? info->disconnect_reason < 256 : info->disconnect_reason == 0))
					break;
				if (pDATA->staLinked)
					pDATA->staLost = true;
				pDATA->staLinked = false;
				pDATA->staHasIp	 = false;
				wifiUpdateSleep();
				wifiEventSend(ARDUINO_EVENT_WIFI_STA_DISCONNECTED, info ? reasonToArduino(info->disconnect_reason) : 0);
				break;
			}
			case EVENT_WIFI_AP_CONNECTED:
				wifiEventSend(ARDUINO_EVENT_WIFI_AP_STACONNECTED, 0);
				break;
			case EVENT_WIFI_AP_DISCONNECTED:
				wifiEventSend(ARDUINO_EVENT_WIFI_AP_STADISCONNECTED, 0);
				break;
		}
	} else if (module == EVENT_MOD_NETIF && event_id == EVENT_NETIF_GOT_IP4) {
		netif_event_got_ip4_t *info = (netif_event_got_ip4_t *)event_data;
		if (!info || info->netif_if == NETIF_IF_STA) {
			pDATA->staHasIp = true;
			// ARMINO ignores power save requests while the SoftAP runs; apply it again when connected
			if (pDATA->sleep)
				bk_wifi_sta_pm_enable();
			wifiUpdateSleep();
			wifiEventSend(ARDUINO_EVENT_WIFI_STA_GOT_IP, 0);
		}
	}
	return BK_OK;
}

void startWifiTask() {
	if (!wifiEventQueueHandle)
		wifiEventQueueHandle = xQueueCreate(32, sizeof(WiFiEventMsg));
	if (!wifiEventTaskHandle) {
		xTaskCreate(wifiEventTask, "wifievent", 1024, NULL, 4, &wifiEventTaskHandle);
		bk_event_register_cb(EVENT_MOD_WIFI, EVENT_ID_ALL, wifiEventCallback, NULL);
		bk_event_register_cb(EVENT_MOD_NETIF, EVENT_NETIF_GOT_IP4, wifiEventCallback, NULL);
	}
}
