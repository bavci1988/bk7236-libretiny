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

	disconnect(false);

	strncpy(DATA->ssid, ssid, sizeof(DATA->ssid) - 1);
	DATA->ssid[sizeof(DATA->ssid) - 1] = '\0';
	if (passphrase) {
		strncpy(DATA->pass, passphrase, sizeof(DATA->pass) - 1);
		DATA->pass[sizeof(DATA->pass) - 1] = '\0';
	} else {
		DATA->pass[0] = '\0';
	}
	DATA->link.channel = channel;

	if (connect && reconnect(bssid))
		return WL_CONNECTED;
	return connect ? WL_CONNECT_FAILED : WL_IDLE_STATUS;
}

bool WiFiClass::config(IPAddress localIP, IPAddress gateway, IPAddress subnet, IPAddress dns1, IPAddress dns2) {
	DATA->useStaticIp = (uint32_t)localIP != 0;
	if (DATA->useStaticIp) {
		DATA->staticIp.ip	   = localIP;
		DATA->staticIp.mask	   = subnet;
		DATA->staticIp.gateway = gateway;
		DATA->staticIp.dns	   = dns1;
	}
	if (DATA->staEnabled)
		return ltw_sta_set_ip4(DATA->useStaticIp ? &DATA->staticIp : NULL);
	return true;
}

bool WiFiClass::reconnect(const uint8_t *bssid) {
	if (!bssid && !DATA->ssid[0]) {
		LT_EM(WIFI, "(B)SSID not specified");
		return false;
	}
	if (bssid) {
		LT_IM(WIFI, "Connecting to " MACSTR, MAC2STR(bssid));
	} else {
		LT_IM(WIFI, "Connecting to %s", DATA->ssid);
	}
	if (DATA->useStaticIp) {
		ltw_sta_set_ip4(&DATA->staticIp);
	}
	DATA->staStatus = WL_DISCONNECTED;
	if (!ltw_sta_connect(DATA->ssid, DATA->pass, DATA->link.channel, bssid)) {
		LT_EM(WIFI, "Starting the connection failed");
		DATA->staStatus = WL_CONNECT_FAILED;
		return false;
	}
	return true;
}

bool WiFiClass::disconnect(bool wifiOff) {
	if (DATA->staEnabled)
		ltw_sta_disconnect();
	DATA->staStatus = WL_DISCONNECTED;
	if (wifiOff)
		enableSTA(false);
	return true;
}

bool WiFiClass::setAutoReconnect(bool autoReconnect) {
	return false;
}

bool WiFiClass::getAutoReconnect() {
	return false;
}

IPAddress WiFiClass::localIP() {
	ltw_ip4_t ip;
	if (!ltw_get_ip4(false, &ip))
		return IPAddress();
	return IPAddress(ip.ip);
}

IPAddress WiFiClass::subnetMask() {
	ltw_ip4_t ip;
	if (!ltw_get_ip4(false, &ip))
		return IPAddress();
	return IPAddress(ip.mask);
}

IPAddress WiFiClass::gatewayIP() {
	ltw_ip4_t ip;
	if (!ltw_get_ip4(false, &ip))
		return IPAddress();
	return IPAddress(ip.gateway);
}

IPAddress WiFiClass::dnsIP(uint8_t dns_no) {
	ltw_ip4_t ip;
	if (dns_no != 0 || !ltw_get_ip4(false, &ip))
		return IPAddress();
	return IPAddress(ip.dns);
}

IPAddress WiFiClass::broadcastIP() {
	return calculateBroadcast(localIP(), subnetMask());
}

const char *WiFiClass::getHostname() {
	struct netif *ifs = (struct netif *)ltw_get_netif(false);
	if (!ifs)
		return NULL;
	return netif_get_hostname(ifs);
}

bool WiFiClass::setHostname(const char *hostname) {
	struct netif *ifs = (struct netif *)ltw_get_netif(false);
	if (!ifs)
		return false;
	netif_set_hostname(ifs, (char *)storeHostname(staHostname, hostname));
	return true;
}

uint8_t *WiFiClass::macAddress(uint8_t *mac) {
	ltw_get_mac(false, mac);
	return mac;
}

bool WiFiClass::setMacAddress(const uint8_t *mac) {
	if (mac[0] & 0x01) {
		LT_EM(WIFI, "Invalid MAC address");
		return false;
	}
	return ltw_set_mac(mac);
}

const String WiFiClass::SSID() {
	if (!ltw_sta_get_link(&DATA->link))
		return "";
	return DATA->link.ssid;
}

const String WiFiClass::psk() {
	if (!isConnected())
		return "";
	return DATA->pass;
}

uint8_t *WiFiClass::BSSID() {
	if (!ltw_sta_get_link(&DATA->link))
		return NULL;
	return DATA->link.bssid;
}

int32_t WiFiClass::channel() {
	if (!ltw_sta_get_link(&DATA->link))
		return 0;
	return DATA->link.channel;
}

int8_t WiFiClass::RSSI() {
	if (!ltw_sta_get_link(&DATA->link))
		return 0;
	return DATA->link.rssi;
}

WiFiAuthMode WiFiClass::getEncryption() {
	if (!ltw_sta_get_link(&DATA->link))
		return WIFI_AUTH_INVALID;
	return (WiFiAuthMode)DATA->link.auth;
}
