#include "WiFiPrivate.h"

// called from the sys_ctrl task (WiFiEvents.cpp)
void wifiScanDone(bool success) {
	WiFiClass *cls	   = (WiFiClass *)pWiFi;
	WiFiScanData *scan = cls->scan;
	if (!scan || !scan->running)
		// discarded (timeout)
		return;

	int count = 0;
	bool ap	  = cDATA->mode == WIFI_MODE_AP;
	if (success && (ap ? wlan_ap_get_scan_result_num(&count) : wlan_sta_get_scan_result_num(&count)) == 0 &&
		count > 0) {
		wlan_sta_ap_t *aps				= (wlan_sta_ap_t *)calloc(count, sizeof(wlan_sta_ap_t));
		wlan_sta_scan_results_t results = {
			.ap	  = aps,
			.size = count,
		};
		if (aps && (ap ? wlan_ap_scan_result(&results) : wlan_sta_scan_result(&results)) == 0) {
			count = MIN(results.num, count);
			cls->scanAlloc(count);
			for (int i = 0; i < count && i < scan->count; i++) {
				char ssid[WLAN_SSID_MAX_LEN + 1];
				size_t len = MIN(aps[i].ssid.ssid_len, WLAN_SSID_MAX_LEN);
				memcpy(ssid, aps[i].ssid.ssid, len);
				ssid[len]			= '\0';
				scan->ap[i].ssid	= strdup(ssid);
				scan->ap[i].auth	= wpaFlagsToAuthMode(aps[i].wpa_flags);
				scan->ap[i].rssi	= aps[i].level;
				scan->ap[i].channel = aps[i].channel;
				memcpy(scan->ap[i].bssid.addr, aps[i].bssid, 6);
			}
			LT_IM(WIFI, "Found %d APs", scan->count);
		}
		free(aps);
	}

	scan->timeout = 0;
	scan->running = false;
	xSemaphoreGive(cDATA->scanSem);

	EventInfo eventInfo;
	memset(&eventInfo, 0, sizeof(EventInfo));
	eventInfo.wifi_scan_done.status = success ? 0 : 1;
	eventInfo.wifi_scan_done.number = scan->count;
	pWiFi->postEvent(ARDUINO_EVENT_WIFI_SCAN_DONE, eventInfo);
}

int16_t WiFiClass::scanNetworks(bool async, bool showHidden, bool passive, uint32_t maxMsPerChannel, uint8_t channel) {
	if (scan && scan->running) {
		if (scan->timeout && millis() > scan->timeout) {
			LT_WM(WIFI, "Scan timeout, discarding");
			scan->running = false;
		} else {
			return WIFI_SCAN_RUNNING;
		}
	}
	if (DATA->mode != WIFI_MODE_AP)
		enableSTA(true);
	scanDelete();
	scanInit();

	LT_IM(WIFI, "Starting WiFi scan");

	wlan_sta_scan_param_t param = {
		.scan_only	  = 1,
		.scan_passive = passive,
		.scan_ssid	  = 0,
	};
	xSemaphoreTake(DATA->scanSem, 0); // reset the semaphore
	scan->running = true;
	scan->timeout = millis() + maxMsPerChannel * 20 + 1000;
	if ((DATA->mode == WIFI_MODE_AP ? wlan_ap_scan(&param) : wlan_sta_scan(&param)) != 0) {
		scan->running = false;
		return WIFI_SCAN_FAILED;
	}

	if (async)
		return WIFI_SCAN_RUNNING;

	LT_IM(WIFI, "Waiting for results");
	xSemaphoreTake(DATA->scanSem, pdMS_TO_TICKS(maxMsPerChannel * 20));
	if (scan->running) {
		scanDelete();
		return WIFI_SCAN_FAILED;
	}
	return scan->count;
}
