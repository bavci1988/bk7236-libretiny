#include "WiFiPrivate.h"

bool WiFiClass::softAP(const char *ssid, const char *passphrase, int channel, bool ssidHidden, int maxClients) {
	if (!enableAP(true))
		return false;
	if (!validate(ssid, passphrase))
		return false;

	LT_IM(WIFI, "Creating SoftAP %s", ssid);
	if (!DATA->apIp.ip)
		softAPConfig((uint32_t)0, (uint32_t)0, (uint32_t)0);
	ltw_ap_set_ip4(&DATA->apIp);

	strncpy(DATA->apSsid, ssid, sizeof(DATA->apSsid) - 1);
	DATA->apSsid[sizeof(DATA->apSsid) - 1] = '\0';
	if (!ltw_ap_start(ssid, passphrase, channel, ssidHidden, maxClients)) {
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
	DATA->apIp.ip	   = localIP;
	DATA->apIp.gateway = gateway;
	DATA->apIp.mask	   = subnet;
	DATA->apIp.dns	   = gateway;
	if (DATA->apEnabled && DATA->apSsid[0])
		return ltw_ap_set_ip4(&DATA->apIp);
	return true;
}

bool WiFiClass::softAPdisconnect(bool wifiOff) {
	if (!(getMode() & WIFI_MODE_AP))
		return true;
	DATA->apSsid[0] = '\0';
	if (wifiOff)
		return enableAP(false);
	return ltw_ap_stop();
}

uint8_t WiFiClass::softAPgetStationNum() {
	return 0;
}

IPAddress WiFiClass::softAPIP() {
	ltw_ip4_t ip;
	if (!ltw_get_ip4(true, &ip))
		return IPAddress();
	return IPAddress(ip.ip);
}

IPAddress WiFiClass::softAPSubnetMask() {
	ltw_ip4_t ip;
	if (!ltw_get_ip4(true, &ip))
		return IPAddress();
	return IPAddress(ip.mask);
}

const char *WiFiClass::softAPgetHostname() {
	struct netif *ifs = (struct netif *)ltw_get_netif(true);
	if (!ifs)
		return NULL;
	return netif_get_hostname(ifs);
}

bool WiFiClass::softAPsetHostname(const char *hostname) {
	struct netif *ifs = (struct netif *)ltw_get_netif(true);
	if (!ifs)
		return false;
	netif_set_hostname(ifs, (char *)storeHostname(apHostname, hostname));
	return true;
}

uint8_t *WiFiClass::softAPmacAddress(uint8_t *mac) {
	ltw_get_mac(true, mac);
	return mac;
}

String WiFiClass::softAPmacAddress(void) {
	uint8_t mac[6];
	ltw_get_mac(true, mac);
	return macToString(mac);
}

const String WiFiClass::softAPSSID(void) {
	if (!(getMode() & WIFI_MODE_AP))
		return "";
	return DATA->apSsid;
}
