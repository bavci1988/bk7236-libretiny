#include "WiFiPrivate.h"

WiFiStatus WiFiClass::begin(
	const char *ssid,
	const char *passphrase,
	int32_t channel,
	const uint8_t *bssid,
	bool connect
) {
	if (!enableSTA(true))
		return WL_CONNECT_FAILED;
	if (!validate(ssid, passphrase))
		return WL_CONNECT_FAILED;

	WiFiNetworkInfo &info = DATA->sta;
	if (info.ssid != ssid)
		// free network info, if not called from restoreSTAConfig()
		resetNetworkInfo(info);

	if (info.ssid != ssid)
		info.ssid = strdup(ssid);
	info.channel = channel;
	info.auth	 = passphrase ? WIFI_AUTH_WPA2_PSK : WIFI_AUTH_OPEN;
	if (passphrase && info.password != passphrase)
		info.password = strdup(passphrase);

	if (!connect)
		return status();
	// the join has started; like the other families, the result comes with the events
	return reconnect(bssid) ? WL_CONNECTED : WL_CONNECT_FAILED;
}

bool WiFiClass::config(IPAddress localIP, IPAddress gateway, IPAddress subnet, IPAddress dns1, IPAddress dns2) {
	if (!enableSTA(true))
		return false;
	WiFiNetworkInfo &info = DATA->sta;
	info.localIP		  = localIP;
	info.subnet			  = subnet;
	info.gateway		  = gateway;
	info.dns1			  = dns1;
	info.dns2			  = dns2;

	ip_addr_t d;
	if (dns1[0]) {
		ip_addr_set_ip4_u32(&d, dns1);
		dns_setserver(0, &d);
	}
	if (dns2[0]) {
		ip_addr_set_ip4_u32(&d, dns2);
		dns_setserver(1, &d);
	}

	// net_ctrl applies these when the station connects
	struct sysinfo *sysinfo = sysinfo_get();
	if (sysinfo == NULL)
		return false;
	sysinfo->sta_use_dhcp = !localIP[0];
	if (localIP[0]) {
		ip4_addr_set_u32(&sysinfo->netif_sta_param.ip_addr, localIP);
		ip4_addr_set_u32(&sysinfo->netif_sta_param.net_mask, subnet);
		ip4_addr_set_u32(&sysinfo->netif_sta_param.gateway, gateway);
	}
	return true;
}

bool WiFiClass::reconnect(const uint8_t *bssid) {
	WiFiNetworkInfo &info = DATA->sta;
	if (!info.ssid)
		return false;
	LT_IM(WIFI, "Connecting to %s", info.ssid);

	// wlan_sta_set() takes mutable buffers
	uint8_t ssid[WLAN_SSID_MAX_LEN] = {0};
	uint8_t psk[65]					= {0};
	size_t ssid_len					= MIN(strlen(info.ssid), sizeof(ssid));
	memcpy(ssid, info.ssid, ssid_len);
	if (info.password)
		strncpy((char *)psk, info.password, sizeof(psk) - 1);

	if (DATA->staEnabled && wlan_sta_disable() != 0)
		return false;
	DATA->staEnabled = false;
	if (wlan_sta_set(ssid, ssid_len, info.password ? psk : NULL) != 0)
		return false;
	if (wlan_sta_enable() != 0)
		return false;
	DATA->staEnabled = true;
	// after wlan_sta_disconnect() (WiFi.disconnect()), enabling the station does not join again
	wlan_sta_connect();
	return true;
}

bool WiFiClass::disconnect(bool wifiOff) {
	int ret = wlan_sta_disconnect();
	if (wifiOff)
		enableSTA(false);
	return ret == 0;
}

bool WiFiClass::setAutoReconnect(bool autoReconnect) {
	return wlan_sta_set_autoconnect(autoReconnect) == 0;
}

bool WiFiClass::getAutoReconnect() {
	return false;
}

IPAddress WiFiClass::localIP() {
	struct netif *nif = staNetif();
	return nif ? IPAddress(netif_ip4_addr(nif)->addr) : IPAddress();
}

IPAddress WiFiClass::subnetMask() {
	struct netif *nif = staNetif();
	return nif ? IPAddress(netif_ip4_netmask(nif)->addr) : IPAddress();
}

IPAddress WiFiClass::gatewayIP() {
	struct netif *nif = staNetif();
	return nif ? IPAddress(netif_ip4_gw(nif)->addr) : IPAddress();
}

IPAddress WiFiClass::dnsIP(uint8_t dns_no) {
	return ip_addr_get_ip4_u32(dns_getserver(dns_no));
}

IPAddress WiFiClass::broadcastIP() {
	return calculateBroadcast(localIP(), subnetMask());
}

const char *WiFiClass::getHostname() {
	struct netif *nif = staNetif();
	return nif ? netif_get_hostname(nif) : NULL;
}

bool WiFiClass::setHostname(const char *hostname) {
	struct netif *nif = staNetif();
	if (nif == NULL)
		return false;
	netif_set_hostname(nif, (char *)storeHostname(staHostname, hostname));
	return true;
}

uint8_t *WiFiClass::macAddress(uint8_t *mac) {
	struct netif *nif = staNetif();
	if (nif == NULL || wlan_get_mac_addr(nif, mac, 6) != 6)
		lt_get_device_mac(mac);
	return mac;
}

bool WiFiClass::setMacAddress(const uint8_t *mac) {
	struct netif *nif = staNetif();
	return nif && wlan_set_mac_addr(nif, (uint8_t *)mac, 6) == 0;
}

const String WiFiClass::SSID() {
	wlan_sta_ap_t ap;
	if (!isConnected() || wlan_sta_ap_info(&ap) != 0)
		return "";
	char ssid[WLAN_SSID_MAX_LEN + 1];
	size_t len = MIN(ap.ssid.ssid_len, WLAN_SSID_MAX_LEN);
	memcpy(ssid, ap.ssid.ssid, len);
	ssid[len] = '\0';
	return ssid;
}

const String WiFiClass::psk() {
	if (!isConnected() || !DATA->sta.password)
		return "";
	return DATA->sta.password;
}

uint8_t *WiFiClass::BSSID() {
	static uint8_t bssid[6];
	wlan_sta_ap_t ap;
	if (wlan_sta_ap_info(&ap) != 0)
		return NULL;
	memcpy(bssid, ap.bssid, sizeof(bssid));
	return bssid;
}

int32_t WiFiClass::channel() {
	wlan_sta_ap_t ap;
	return wlan_sta_ap_info(&ap) == 0 ? ap.channel : 0;
}

int8_t WiFiClass::RSSI() {
	struct netif *nif = staNetif();
	if (nif != NULL) {
		wlan_ext_signal_t signal;
		if (wlan_ext_request(nif, WLAN_EXT_CMD_GET_SIGNAL, (uint32_t)&signal) == 0) {
			int level = signal.rssi / 2 + signal.noise;
			if (level < 0 && level > -110)
				return level;
		}
	}
	wlan_sta_ap_t ap;
	return wlan_sta_ap_info(&ap) == 0 ? ap.level : 0;
}

WiFiAuthMode WiFiClass::getEncryption() {
	wlan_sta_ap_t ap;
	return wlan_sta_ap_info(&ap) == 0 ? wpaFlagsToAuthMode(ap.wpa_flags) : WIFI_AUTH_INVALID;
}
