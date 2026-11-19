/* Copyright (c) Kuba Szczodrzyński 2022-06-26. */

#include "WiFiPrivate.h"

WiFiClass::WiFiClass() {
	data			= (WiFiData *)calloc(1, sizeof(WiFiData));
	DATA->scanSem	= xSemaphoreCreateBinary();
	DATA->staStatus = WL_IDLE_STATUS;
}

WiFiClass::~WiFiClass() {
	vSemaphoreDelete(DATA->scanSem);
	free(data);
	data = NULL;
}

WiFiAuthMode securityToAuthMode(wifi_security_t security) {
	switch (security) {
		case WIFI_SECURITY_NONE:
			return WIFI_AUTH_OPEN;
		case WIFI_SECURITY_WEP:
			return WIFI_AUTH_WEP;
		case WIFI_SECURITY_WPA_TKIP:
		case WIFI_SECURITY_WPA_AES:
			return WIFI_AUTH_WPA_PSK;
		case WIFI_SECURITY_WPA_MIXED:
			return WIFI_AUTH_WPA_WPA2_PSK;
		case WIFI_SECURITY_WPA2_TKIP:
		case WIFI_SECURITY_WPA2_AES:
		case WIFI_SECURITY_WPA2_MIXED:
			return WIFI_AUTH_WPA2_PSK;
		case WIFI_SECURITY_WPA3_SAE:
			return WIFI_AUTH_WPA3_PSK;
		case WIFI_SECURITY_WPA3_WPA2_MIXED:
			return WIFI_AUTH_WPA2_WPA3_PSK;
		case WIFI_SECURITY_EAP:
			return WIFI_AUTH_WPA2_ENTERPRISE;
		case WIFI_SECURITY_TYPE_WAPI_PSK:
			return WIFI_AUTH_WAPI_PSK;
		default:
			return WIFI_AUTH_INVALID;
	}
}

// ARMINO's netif API takes IP addresses as strings
static void ipToString(char *str, IPAddress ip) {
	snprintf(str, NETIF_IP4_STR_LEN, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
}

void ipToConfig(netif_ip4_config_t *config, IPAddress ip, IPAddress mask, IPAddress gateway, IPAddress dns) {
	memset(config, 0, sizeof(*config));
	ipToString(config->ip, ip);
	ipToString(config->mask, mask);
	ipToString(config->gateway, gateway);
	ipToString(config->dns, dns);
}

IPAddress configToIp(const char *str) {
	IPAddress ip;
	if (!ip.fromString(str))
		return IPAddress();
	return ip;
}
