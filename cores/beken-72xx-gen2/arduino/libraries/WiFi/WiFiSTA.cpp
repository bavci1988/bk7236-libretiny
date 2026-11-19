/* Copyright (c) Kuba Szczodrzyński 2022-06-26. */

#include "WiFiPrivate.h"

static bool getLink(WiFiData *data) {
	memset(&data->link, 0, sizeof(data->link));
	if (bk_wifi_sta_get_link_status(&data->link) != BK_OK)
		return false;
	return data->link.state == WIFI_LINKSTATE_STA_GOT_IP || data->link.state == WIFI_LINKSTATE_STA_CONNECTED;
}

static bool getIp(netif_ip4_config_t *config) {
	memset(config, 0, sizeof(*config));
	return bk_netif_get_ip4_config(NETIF_IF_STA, config) == BK_OK;
}

static bool applyIp(WiFiData *data) {
	if (!data->useStaticIp)
		return bk_netif_dhcpc_start(NETIF_IF_STA) == BK_OK;
	return bk_netif_set_ip4_config(NETIF_IF_STA, &data->staIp) == BK_OK;
}

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
	DATA->channel = channel;

	if (connect && reconnect(bssid))
		return WL_CONNECTED;
	return connect ? WL_CONNECT_FAILED : WL_IDLE_STATUS;
}

bool WiFiClass::config(IPAddress localIP, IPAddress gateway, IPAddress subnet, IPAddress dns1, IPAddress dns2) {
	DATA->useStaticIp = (uint32_t)localIP != 0;
	if (DATA->useStaticIp)
		ipToConfig(&DATA->staIp, localIP, subnet, gateway, dns1);
	if (DATA->staEnabled)
		return applyIp(DATA);
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
	if (DATA->useStaticIp)
		applyIp(DATA);
	DATA->staStatus = WL_DISCONNECTED;

	wifi_sta_config_t config;
	memset(&config, 0, sizeof(config));
	config.security = WIFI_SECURITY_AUTO;
	strncpy(config.ssid, DATA->ssid, WIFI_SSID_STR_LEN - 1);
	strncpy(config.password, DATA->pass, WIFI_PASSWORD_LEN - 1);
	config.channel = DATA->channel;
	if (bssid)
		memcpy(config.bssid, bssid, WIFI_BSSID_LEN);
	if (bk_wifi_sta_set_config(&config) != BK_OK || bk_wifi_sta_start() != BK_OK) {
		LT_EM(WIFI, "Starting the connection failed");
		DATA->staStatus = WL_CONNECT_FAILED;
		return false;
	}
	return true;
}

bool WiFiClass::disconnect(bool wifiOff) {
	if (DATA->staEnabled) {
		if (getLink(DATA))
			// disconnect an existing connection
			bk_wifi_sta_disconnect();
		else
			// abort a pending connection attempt (ARMINO keeps retrying otherwise);
			// a new connection is started by reconnect()
			bk_wifi_sta_stop();
	}
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
	netif_ip4_config_t config;
	if (!getIp(&config))
		return IPAddress();
	return configToIp(config.ip);
}

IPAddress WiFiClass::subnetMask() {
	netif_ip4_config_t config;
	if (!getIp(&config))
		return IPAddress();
	return configToIp(config.mask);
}

IPAddress WiFiClass::gatewayIP() {
	netif_ip4_config_t config;
	if (!getIp(&config))
		return IPAddress();
	return configToIp(config.gateway);
}

IPAddress WiFiClass::dnsIP(uint8_t dns_no) {
	netif_ip4_config_t config;
	if (dns_no != 0 || !getIp(&config))
		return IPAddress();
	return configToIp(config.dns);
}

IPAddress WiFiClass::broadcastIP() {
	return calculateBroadcast(localIP(), subnetMask());
}

const char *WiFiClass::getHostname() {
	struct netif *ifs = (struct netif *)net_get_sta_handle();
	if (!ifs)
		return NULL;
	return netif_get_hostname(ifs);
}

bool WiFiClass::setHostname(const char *hostname) {
	struct netif *ifs = (struct netif *)net_get_sta_handle();
	if (!ifs)
		return false;
	netif_set_hostname(ifs, (char *)storeHostname(staHostname, hostname));
	return true;
}

uint8_t *WiFiClass::macAddress(uint8_t *mac) {
	bk_get_mac(mac, MAC_TYPE_STA);
	return mac;
}

bool WiFiClass::setMacAddress(const uint8_t *mac) {
	if (mac[0] & 0x01) {
		LT_EM(WIFI, "Invalid MAC address");
		return false;
	}
	return bk_set_base_mac(mac) == BK_OK;
}

const String WiFiClass::SSID() {
	if (!getLink(DATA))
		return "";
	return DATA->link.ssid;
}

const String WiFiClass::psk() {
	if (!isConnected())
		return "";
	return DATA->pass;
}

uint8_t *WiFiClass::BSSID() {
	if (!getLink(DATA))
		return NULL;
	return DATA->link.bssid;
}

int32_t WiFiClass::channel() {
	if (!getLink(DATA))
		return 0;
	return DATA->link.channel;
}

int8_t WiFiClass::RSSI() {
	if (!getLink(DATA))
		return 0;
	return DATA->link.rssi;
}

WiFiAuthMode WiFiClass::getEncryption() {
	if (!getLink(DATA))
		return WIFI_AUTH_INVALID;
	return securityToAuthMode(DATA->link.security);
}
