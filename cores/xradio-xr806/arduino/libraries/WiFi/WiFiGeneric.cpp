#include "WiFiPrivate.h"

// the AP must be fully stopped before the station starts
static bool leaveAP(WiFiClass *cls) {
	if (cDATA->apEnabled && wlan_ap_disable() != 0)
		return false;
	cDATA->apEnabled = false;
	// net_switch_mode() does not stop the DHCP server; the station netif reuses the AP netif name,
	// so the server would otherwise answer DHCP requests on the station network
	dhcp_server_stop();
	return net_switch_mode(WLAN_MODE_STA) == 0;
}

bool WiFiClass::modePriv(WiFiMode mode, WiFiModeAction sta, WiFiModeAction ap) {
	if (!DATA->initialized) {
		// platform_init() starts the WLAN core and the station netif
		if (wlan_netif_get(WLAN_MODE_NONE) == NULL) {
			LT_EM(WIFI, "WLAN core did not start");
			return false;
		}
		if (!registerWifiHandlers())
			LT_WM(WIFI, "Cannot attach the WLAN event observer");
		DATA->mode		  = WIFI_MODE_STA;
		DATA->initialized = true;
	}

	if (mode == WIFI_MODE_APSTA) {
		LT_EM(WIFI, "AP+STA mode not supported!");
		return false;
	}

	if (ap == WLMODE_DISABLE && DATA->mode == WIFI_MODE_AP) {
		LT_DM(WIFI, "Disabling AP");
		if (!leaveAP(this))
			return false;
		DATA->mode = WIFI_MODE_STA;
		wifiEventSendArduino(ARDUINO_EVENT_WIFI_AP_STOP);
	}
	if (sta == WLMODE_DISABLE && DATA->staEnabled) {
		LT_DM(WIFI, "Disabling STA");
		if (wlan_sta_disable() != 0)
			return false;
		DATA->staEnabled = false;
		wifiEventSendArduino(ARDUINO_EVENT_WIFI_STA_STOP);
	}
	if (ap == WLMODE_ENABLE && DATA->mode != WIFI_MODE_AP) {
		LT_DM(WIFI, "Enabling AP");
		if (DATA->staEnabled && wlan_sta_disable() != 0)
			return false;
		DATA->staEnabled = false;
		if (net_switch_mode(WLAN_MODE_HOSTAP) != 0)
			return false;
		// net_switch_mode() starts the AP with the SDK default SSID; softAP() sets it up again
		wlan_ap_disable();
		DATA->mode = WIFI_MODE_AP;
	}
	if (sta == WLMODE_ENABLE && !DATA->staEnabled) {
		LT_DM(WIFI, "Enabling STA");
		if (DATA->mode == WIFI_MODE_AP && !leaveAP(this))
			return false;
		DATA->mode = WIFI_MODE_STA;
		wifiEventSendArduino(ARDUINO_EVENT_WIFI_STA_START);
	}
	if (mode == WIFI_MODE_NULL)
		DATA->mode = WIFI_MODE_NULL;
	return true;
}

WiFiMode WiFiClass::getMode() {
	if (!DATA->initialized)
		return WIFI_MODE_NULL;
	return DATA->mode;
}

WiFiStatus WiFiClass::status() {
	if (!DATA->initialized || DATA->mode != WIFI_MODE_STA)
		return WL_DISCONNECTED;
	wlan_sta_states_t state = WLAN_STA_STATE_DISCONNECTED;
	if (wlan_sta_state(&state) != 0 || state != WLAN_STA_STATE_CONNECTED)
		return WL_DISCONNECTED;
	struct netif *nif = staNetif();
	if (nif == NULL || !netif_is_up(nif) || ip4_addr_isany(netif_ip4_addr(nif)))
		return WL_IDLE_STATUS;
	return WL_CONNECTED;
}

bool WiFiClass::setSleep(bool enable) {
	LT_DM(WIFI, "WiFi sleep mode %u", enable);
	// applied again on every connection; the SDK switches it on then
	DATA->sleep = enable;
	return xr806_wlan_apply_power_save(staNetif(), enable);
}

bool WiFiClass::getSleep() {
	return DATA->sleep;
}

IPAddress WiFiClass::hostByName(const char *hostname) {
	ip_addr_t ip;
	if (netconn_gethostbyname(hostname, &ip) == ERR_OK)
		return ip_addr_get_ip4_u32(&ip);
	return IPAddress();
}
