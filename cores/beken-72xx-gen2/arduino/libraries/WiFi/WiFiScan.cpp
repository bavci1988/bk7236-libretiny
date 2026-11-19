/* Copyright (c) Kuba Szczodrzyński 2022-06-26. */

#include "WiFiPrivate.h"

// called from the WiFi event task (WiFiEvents.cpp)
void wifiScanDone(WiFiClass *cls) {
	WiFiScanData *scan = cls->scan;
	if (!scan)
		return;

	wifi_scan_result_t result = {0};
	if (bk_wifi_scan_get_result(&result) != BK_OK)
		result.ap_num = 0;
	LT_IM(WIFI, "Found %d APs", result.ap_num);
	uint8_t apNum = cls->scanAlloc(result.ap_num);
	if (apNum < result.ap_num)
		LT_WM(WIFI, "Alloc failed, only %d APs will be copied", apNum);
	for (uint8_t i = 0; i < apNum; i++) {
		scan->ap[i].ssid	= strdup(result.aps[i].ssid);
		scan->ap[i].auth	= securityToAuthMode(result.aps[i].security);
		scan->ap[i].rssi	= result.aps[i].rssi;
		scan->ap[i].channel = result.aps[i].channel;
		memcpy(scan->ap[i].bssid.addr, result.aps[i].bssid, 6);
	}
	bk_wifi_scan_free_result(&result);

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
	if (bk_wifi_scan_start(NULL) != BK_OK) {
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
