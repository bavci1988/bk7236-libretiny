#include "WiFiPrivate.h"

typedef struct {
	uint16_t event; // ltw_event_t or (WIFI_EVENT_ARDUINO | EventId)
	uint16_t reason;
} WiFiEventMsg;

#define WIFI_EVENT_ARDUINO (1 << 15)

extern void wifiScanDone(WiFiClass *cls);

static QueueHandle_t wifiEventQueueHandle = NULL;
static TaskHandle_t wifiEventTaskHandle	  = NULL;

static void wifiEventHandler(WiFiEventMsg msg) {
	if (!pWiFi)
		return; // failsafe
	LT_DM(WIFI, "Event %u (reason %u)", msg.event, msg.reason);

	EventId eventId;
	EventInfo eventInfo;
	memset(&eventInfo, 0, sizeof(EventInfo));

	if (msg.event & WIFI_EVENT_ARDUINO) {
		pWiFi->postEvent((EventId)(msg.event & ~WIFI_EVENT_ARDUINO), eventInfo);
		return;
	}

	switch (msg.event) {
		case LTW_EVENT_SCAN_DONE:
			wifiScanDone(pWiFi);
			eventId							= ARDUINO_EVENT_WIFI_SCAN_DONE;
			eventInfo.wifi_scan_done.status = 0;
			if (pWiFi->scan)
				eventInfo.wifi_scan_done.number = pWiFi->scan->count;
			break;
		case LTW_EVENT_STA_CONNECTED: {
			eventId			 = ARDUINO_EVENT_WIFI_STA_CONNECTED;
			String ssid		 = pWiFi->SSID();
			uint8_t *bssid	 = pWiFi->BSSID();
			size_t ssidLen	 = min(ssid.length(), sizeof(eventInfo.wifi_sta_connected.ssid) - 1);
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
		case LTW_EVENT_STA_DISCONNECTED:
			eventId								   = ARDUINO_EVENT_WIFI_STA_DISCONNECTED;
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
		case LTW_EVENT_STA_GOT_IP:
			eventId								  = ARDUINO_EVENT_WIFI_STA_GOT_IP;
			eventInfo.got_ip.if_index			  = 0;
			eventInfo.got_ip.ip_changed			  = true;
			eventInfo.got_ip.ip_info.ip.addr	  = pWiFi->localIP();
			eventInfo.got_ip.ip_info.netmask.addr = pWiFi->subnetMask();
			eventInfo.got_ip.ip_info.gw.addr	  = pWiFi->gatewayIP();
			pDATA->staStatus					  = WL_CONNECTED;
			break;
		case LTW_EVENT_AP_STA_CONNECTED:
			eventId = ARDUINO_EVENT_WIFI_AP_STACONNECTED;
			break;
		case LTW_EVENT_AP_STA_DISCONNECTED:
			eventId = ARDUINO_EVENT_WIFI_AP_STADISCONNECTED;
			break;
		default:
			return;
	}
	pWiFi->postEvent(eventId, eventInfo);
}

static void wifiEventTask(void *arg) {
	WiFiEventMsg msg;
	for (;;) {
		if (xQueueReceive(wifiEventQueueHandle, &msg, portMAX_DELAY) == pdTRUE)
			wifiEventHandler(msg);
	}
}

static void wifiEventSend(WiFiEventMsg msg) {
	if (wifiEventQueueHandle && wifiEventTaskHandle) {
		xQueueSend(wifiEventQueueHandle, &msg, portMAX_DELAY);
	} else {
		wifiEventHandler(msg);
	}
}

// called from the ARMINO event task (wifi_armino.c)
static void wifiArminoEvent(ltw_event_t event, uint16_t reason) {
	wifiEventSend({.event = (uint16_t)event, .reason = reason});
}

void wifiEventSendArduino(EventId event) {
	wifiEventSend({.event = (uint16_t)(WIFI_EVENT_ARDUINO | event), .reason = 0});
}

void startWifiTask() {
	if (!wifiEventQueueHandle)
		wifiEventQueueHandle = xQueueCreate(32, sizeof(WiFiEventMsg));
	if (!wifiEventTaskHandle)
		xTaskCreate(wifiEventTask, "wifievent", 1024, NULL, 4, &wifiEventTaskHandle);
	ltw_init(wifiArminoEvent);
}
