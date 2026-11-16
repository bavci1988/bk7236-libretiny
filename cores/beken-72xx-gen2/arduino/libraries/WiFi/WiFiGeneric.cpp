#include "WiFiPrivate.h"

bool WiFiClass::modePriv(WiFiMode mode, WiFiModeAction sta, WiFiModeAction ap) {
	startWifiTask();

	if (sta == WLMODE_ENABLE) {
		LT_DM(WIFI, "Enabling STA");
		DATA->staEnabled = true;
		ltw_set_sleep(DATA->sleep);
		wifiEventSendArduino(ARDUINO_EVENT_WIFI_STA_START);
	} else if (sta == WLMODE_DISABLE) {
		LT_DM(WIFI, "Disabling STA");
		ltw_sta_stop();
		DATA->staEnabled = false;
		DATA->staStatus	 = WL_DISCONNECTED;
		wifiEventSendArduino(ARDUINO_EVENT_WIFI_STA_STOP);
	}

	if (ap == WLMODE_ENABLE) {
		LT_DM(WIFI, "Enabling AP");
		DATA->apEnabled = true;
		wifiEventSendArduino(ARDUINO_EVENT_WIFI_AP_START);
	} else if (ap == WLMODE_DISABLE) {
		LT_DM(WIFI, "Disabling AP");
		ltw_ap_stop();
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

bool WiFiClass::setSleep(bool enable) {
	LT_DM(WIFI, "WiFi sleep mode %u", enable);
	DATA->sleep = enable;
	if (DATA->staEnabled)
		return ltw_set_sleep(enable);
	return true;
}

bool WiFiClass::getSleep() {
	return DATA->sleep;
}
