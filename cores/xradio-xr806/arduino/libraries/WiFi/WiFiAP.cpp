#include "WiFiPrivate.h"

bool WiFiClass::softAP(const char *ssid, const char *passphrase, int channel, bool ssidHidden, int maxClients) {
	if (!enableAP(true))
		return false;
	if (!validate(ssid, passphrase))
		return false;

	WiFiNetworkInfo &info = DATA->ap;
	if (info.ssid != ssid)
		// free network info, if not called from restoreAPConfig()
		resetNetworkInfo(info);

	if (info.ssid != ssid)
		info.ssid = strdup(ssid);
	if (passphrase && info.password != passphrase)
		info.password = strdup(passphrase);
	info.ssidHidden = ssidHidden;
	info.channel	= channel;
	info.auth		= passphrase ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN;

	LT_IM(WIFI, "Creating SoftAP %s", ssid);

	uint8_t ssid_buf[WLAN_SSID_MAX_LEN] = {0};
	uint8_t psk[65]						= {0};
	size_t ssid_len						= MIN(strlen(ssid), sizeof(ssid_buf));
	memcpy(ssid_buf, ssid, ssid_len);
	if (passphrase)
		strncpy((char *)psk, passphrase, sizeof(psk) - 1);

	// wlan_ap_enable() fails on a running AP
	wlan_ap_disable();
	DATA->apEnabled = false;
	if (wlan_ap_set(ssid_buf, ssid_len, passphrase ? psk : NULL) != 0)
		return false;
	if (channel > 0) {
		wlan_ap_config_t cfg = {};
		cfg.field			 = WLAN_AP_FIELD_CHANNEL;
		cfg.u.channel		 = channel;
		if (wlan_ap_set_config(&cfg) != 0)
			return false;
	}
	if (maxClients > 0) {
		wlan_ap_config_t cfg = {};
		cfg.field			 = WLAN_AP_FIELD_MAX_NUM_STA;
		cfg.u.max_num_sta	 = maxClients;
		wlan_ap_set_config(&cfg);
	}
	if (wlan_ap_enable() != 0)
		return false;
	DATA->apEnabled = true;
	wifiEventSendArduino(ARDUINO_EVENT_WIFI_AP_START);
	return true;
}

bool WiFiClass::softAPConfig(IPAddress localIP, IPAddress gateway, IPAddress subnet) {
	if (!enableAP(true))
		return false;
	WiFiNetworkInfo &info = DATA->ap;
	info.localIP		  = localIP;
	info.gateway		  = gateway;
	info.subnet			  = subnet;
	// net_ctrl brings the AP netif up with these and starts the DHCP server
	struct sysinfo *sysinfo = sysinfo_get();
	if (sysinfo == NULL)
		return false;
	ip4_addr_set_u32(&sysinfo->netif_ap_param.ip_addr, localIP);
	ip4_addr_set_u32(&sysinfo->netif_ap_param.net_mask, subnet);
	ip4_addr_set_u32(&sysinfo->netif_ap_param.gateway, gateway);
	return true;
}

bool WiFiClass::softAPdisconnect(bool wifiOff) {
	return enableAP(false);
}

uint8_t WiFiClass::softAPgetStationNum() {
	int num = 0;
	if (!DATA->apEnabled || wlan_ap_sta_num(&num) != 0)
		return 0;
	return num;
}

IPAddress WiFiClass::softAPIP() {
	struct netif *nif = apNetif();
	if (nif != NULL && !ip4_addr_isany(netif_ip4_addr(nif)))
		return IPAddress(netif_ip4_addr(nif)->addr);
	// net_ctrl sets the address only after wlan_ap_enable() returns, on its own task
	struct sysinfo *sysinfo = sysinfo_get();
	if (!DATA->apEnabled || sysinfo == NULL)
		return IPAddress();
	return IPAddress(ip4_addr_get_u32(&sysinfo->netif_ap_param.ip_addr));
}

IPAddress WiFiClass::softAPSubnetMask() {
	struct netif *nif = apNetif();
	return nif ? IPAddress(netif_ip4_netmask(nif)->addr) : IPAddress();
}

const char *WiFiClass::softAPgetHostname() {
	struct netif *nif = apNetif();
	return nif ? netif_get_hostname(nif) : NULL;
}

bool WiFiClass::softAPsetHostname(const char *hostname) {
	struct netif *nif = apNetif();
	if (nif == NULL)
		return false;
	netif_set_hostname(nif, (char *)storeHostname(apHostname, hostname));
	return true;
}

uint8_t *WiFiClass::softAPmacAddress(uint8_t *mac) {
	struct netif *nif = apNetif();
	if (nif == NULL || wlan_get_mac_addr(nif, mac, 6) != 6)
		lt_get_device_mac(mac);
	return mac;
}

String WiFiClass::softAPmacAddress(void) {
	uint8_t mac[6];
	softAPmacAddress(mac);
	return macToString(mac);
}

const String WiFiClass::softAPSSID(void) {
	return DATA->ap.ssid ? DATA->ap.ssid : "";
}
