/* Copyright (c) Kuba Szczodrzyński 2022-06-26. */

#include "WiFiPrivate.h"

static bool getIp(netif_ip4_config_t *config) {
	memset(config, 0, sizeof(*config));
	return bk_netif_get_ip4_config(NETIF_IF_AP, config) == BK_OK;
}

bool WiFiClass::softAP(const char *ssid, const char *passphrase, int channel, bool ssidHidden, int maxClients) {
	if (!enableAP(true))
		return false;
	if (!validate(ssid, passphrase))
		return false;

	LT_IM(WIFI, "Creating SoftAP %s", ssid);
	if (!DATA->apIp.ip[0])
		softAPConfig((uint32_t)0, (uint32_t)0, (uint32_t)0);
	bk_netif_set_ip4_config(NETIF_IF_AP, &DATA->apIp);

	strncpy(DATA->apSsid, ssid, sizeof(DATA->apSsid) - 1);
	DATA->apSsid[sizeof(DATA->apSsid) - 1] = '\0';

	wifi_ap_config_t config;
	memset(&config, 0, sizeof(config));
	config.security = WIFI_SECURITY_WPA2_MIXED;
	strncpy(config.ssid, ssid, WIFI_SSID_STR_LEN - 1);
	if (passphrase)
		strncpy(config.password, passphrase, WIFI_PASSWORD_LEN - 1);
	config.channel = channel;
	config.hidden  = ssidHidden;
	// ARMINO falls back to its default (with a warning) for values above the supported maximum
	if (maxClients > CONFIG_WIFI_MAC_SUPPORT_STAS_MAX_NUM)
		maxClients = CONFIG_WIFI_MAC_SUPPORT_STAS_MAX_NUM;
	config.max_con = maxClients;

	// the CPU must stay awake for the SoftAP clients
	DATA->apRunning = true;
	wifiUpdateSleep();
	if (bk_wifi_ap_set_config(&config) != BK_OK || bk_wifi_ap_start() != BK_OK) {
		LT_EM(WIFI, "SoftAP failed");
		return false;
	}
	return true;
}

bool WiFiClass::softAPConfig(IPAddress localIP, IPAddress gateway, IPAddress subnet) {
	if (!localIP) {
		localIP = gateway = IPAddress(192, 168, 43, 1);
		subnet			  = IPAddress(255, 255, 255, 0);
	}
	ipToConfig(&DATA->apIp, localIP, subnet, gateway, gateway);
	if (DATA->apEnabled && DATA->apSsid[0])
		return bk_netif_set_ip4_config(NETIF_IF_AP, &DATA->apIp) == BK_OK;
	return true;
}

bool WiFiClass::softAPdisconnect(bool wifiOff) {
	if (!(getMode() & WIFI_MODE_AP))
		return true;
	DATA->apSsid[0] = '\0';
	if (wifiOff)
		return enableAP(false);
	bool ret = bk_wifi_ap_stop() == BK_OK;
	// power save is only possible without the SoftAP
	if (DATA->sleep)
		bk_wifi_sta_pm_enable();
	DATA->apRunning = false;
	wifiUpdateSleep();
	return ret;
}

uint8_t WiFiClass::softAPgetStationNum() {
	wlan_ap_stas_t stas = {0};
	if (bk_wifi_ap_get_sta_list(&stas) != BK_OK)
		stas.num = 0;
	if (stas.sta)
		os_free(stas.sta);
	return stas.num;
}

IPAddress WiFiClass::softAPIP() {
	netif_ip4_config_t config;
	if (!getIp(&config))
		return IPAddress();
	return configToIp(config.ip);
}

IPAddress WiFiClass::softAPSubnetMask() {
	netif_ip4_config_t config;
	if (!getIp(&config))
		return IPAddress();
	return configToIp(config.mask);
}

const char *WiFiClass::softAPgetHostname() {
	struct netif *ifs = (struct netif *)net_get_uap_handle();
	if (!ifs)
		return NULL;
	return netif_get_hostname(ifs);
}

bool WiFiClass::softAPsetHostname(const char *hostname) {
	struct netif *ifs = (struct netif *)net_get_uap_handle();
	if (!ifs)
		return false;
	netif_set_hostname(ifs, (char *)storeHostname(apHostname, hostname));
	return true;
}

uint8_t *WiFiClass::softAPmacAddress(uint8_t *mac) {
	bk_get_mac(mac, MAC_TYPE_AP);
	return mac;
}

String WiFiClass::softAPmacAddress(void) {
	uint8_t mac[6];
	bk_get_mac(mac, MAC_TYPE_AP);
	return macToString(mac);
}

const String WiFiClass::softAPSSID(void) {
	if (!(getMode() & WIFI_MODE_AP))
		return "";
	return DATA->apSsid;
}
