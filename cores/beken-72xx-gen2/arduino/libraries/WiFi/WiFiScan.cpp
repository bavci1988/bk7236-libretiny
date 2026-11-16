#include "WiFiPrivate.h"

// called from the WiFi event task (WiFiEvents.cpp)
void wifiScanDone(WiFiClass *cls) {
	WiFiScanData *scan = cls->scan;
	if (!scan)
		return;

	int count			 = 0;
	ltw_ap_info_t *list	 = ltw_scan_get_result(&count);
	LT_IM(WIFI, "Found %d APs", count);
	uint8_t apNum = cls->scanAlloc(count);
	if (apNum < count)
		LT_WM(WIFI, "Alloc failed, only %d APs will be copied", apNum);
	for (uint8_t i = 0; i < apNum; i++) {
		scan->ap[i].ssid	= strdup(list[i].ssid);
		scan->ap[i].auth	= (WiFiAuthMode)list[i].auth;
		scan->ap[i].rssi	= list[i].rssi;
		scan->ap[i].channel = list[i].channel;
		memcpy(scan->ap[i].bssid.addr, list[i].bssid, 6);
	}
	free(list);

	scan->timeout = 0;
	if (scan->running) {
		scan->running = false;
		xSemaphoreGive(((WiFiData *)cls->data)->scanSem);
	}
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
	enableSTA(true);
	scanDelete();
	scanInit();

	LT_IM(WIFI, "Starting WiFi scan");
	xSemaphoreTake(DATA->scanSem, 0); // reset the semaphore
	scan->running = true;
	scan->timeout = millis() + maxMsPerChannel * 20 + 1000;
	if (!ltw_scan_start()) {
		LT_EM(WIFI, "Starting the scan failed");
		scanDelete();
		return WIFI_SCAN_FAILED;
	}
	if (async)
		return WIFI_SCAN_RUNNING;

	LT_IM(WIFI, "Waiting for results");
	xSemaphoreTake(DATA->scanSem, pdMS_TO_TICKS(maxMsPerChannel * 20 + 1000));
	if (scan->running) {
		scanDelete();
		return WIFI_SCAN_FAILED;
	}
	return scan->count;
}
