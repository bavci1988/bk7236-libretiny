/* Copyright (c) Kuba Szczodrzyński 2022-06-26. */

#include "WiFiPrivate.h"

bool WiFiClass::modePriv(WiFiMode mode, WiFiModeAction sta, WiFiModeAction ap) {
	startWifiTask();

	if (sta == WLMODE_ENABLE) {
		LT_DM(WIFI, "Enabling STA");
		DATA->staEnabled = true;
		setSleep(DATA->sleep);
		wifiEventSendArduino(ARDUINO_EVENT_WIFI_STA_START);
	} else if (sta == WLMODE_DISABLE) {
		LT_DM(WIFI, "Disabling STA");
		bk_wifi_sta_stop();
		DATA->staEnabled = false;
		DATA->staLinked	 = false;
		DATA->staHasIp	 = false;
		wifiUpdateSleep();
		DATA->staStatus	 = WL_DISCONNECTED;
		wifiEventSendArduino(ARDUINO_EVENT_WIFI_STA_STOP);
	}

	if (ap == WLMODE_ENABLE) {
		LT_DM(WIFI, "Enabling AP");
		DATA->apEnabled = true;
		wifiEventSendArduino(ARDUINO_EVENT_WIFI_AP_START);
	} else if (ap == WLMODE_DISABLE) {
		LT_DM(WIFI, "Disabling AP");
		softAPdisconnect(false);
		DATA->apEnabled = false;
		wifiEventSendArduino(ARDUINO_EVENT_WIFI_AP_STOP);
	}
	return true;
}

WiFiMode WiFiClass::getMode() {
	if (!data)
		return WIFI_MODE_NULL;
	uint8_t sta = DATA->staEnabled * WIFI_MODE_STA;
	uint8_t ap	= DATA->apEnabled * WIFI_MODE_AP;
	return (WiFiMode)(sta | ap);
}

WiFiStatus WiFiClass::status() {
	return DATA->staStatus;
}

IPAddress WiFiClass::hostByName(const char *hostname) {
	ip_addr_t ip;
	if (netconn_gethostbyname(hostname, &ip) == ERR_OK)
		return IPAddress(ip_addr_get_ip4_u32(&ip));
	return IPAddress();
}

// LT_AUTO_LIGHT_SLEEP: the CPU may sleep only while the station can be woken by its access point
void wifiUpdateSleep() {
#if LT_AUTO_LIGHT_SLEEP
	if (!pWiFi)
		return; // setSleep() before the first mode change
	static bool allowed = false;
	bool allow			= pDATA->sleep && pDATA->staHasIp && !pDATA->apRunning;
	// own vote (APP1), so that lt_light_sleep_allow() stays with the application;
	// voted on every call, as this runs in several tasks
	bk_pm_module_vote_sleep_ctrl(PM_SLEEP_MODULE_NAME_APP1, allow ? 1 : 0, 0);
	if (allow != allowed)
		LT_IM(WIFI, "Light sleep %s", allow ? "allowed" : "forbidden");
	allowed = allow;
#endif
}

bool WiFiClass::setSleep(bool enable) {
	LT_DM(WIFI, "WiFi sleep mode %u", enable);
	DATA->sleep = enable;
	wifiUpdateSleep();
	if (!DATA->staEnabled)
		return true;
	return (enable ? bk_wifi_sta_pm_enable() : bk_wifi_sta_pm_disable()) == BK_OK;
}

bool WiFiClass::getSleep() {
	return DATA->sleep;
}
