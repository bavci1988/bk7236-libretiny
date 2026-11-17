#include "wifi_armino.h"

#include <libretiny.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <components/event.h>
#include <components/netif.h>
#include <components/system.h>
#include <modules/wifi.h>
#include <os/mem.h>

extern void *net_get_sta_handle(void);
extern void *net_get_uap_handle(void);

// LibreTiny WiFiAuthMode values
#define LT_AUTH_OPEN		  0
#define LT_AUTH_WEP			  1
#define LT_AUTH_WPA_PSK		  2
#define LT_AUTH_WPA2_PSK	  3
#define LT_AUTH_WPA_WPA2_PSK  4
#define LT_AUTH_WPA2_ENT	  5
#define LT_AUTH_WPA3_PSK	  6
#define LT_AUTH_WPA2_WPA3_PSK 7
#define LT_AUTH_WAPI_PSK	  8
#define LT_AUTH_INVALID		  255

// LibreTiny wifi_err_reason_t values (non-802.11)
#define LT_REASON_UNSPECIFIED	 1
#define LT_REASON_BEACON_TIMEOUT 200
#define LT_REASON_NO_AP_FOUND	 201
#define LT_REASON_AUTH_FAIL		 202

static ltw_event_cb_t event_cb = NULL;
static bool sta_linked		   = false;
// WiFi power save requested by WiFi.setSleep()
static bool sta_ps_enabled = false;

static uint8_t security_to_auth(wifi_security_t security) {
	switch (security) {
		case WIFI_SECURITY_NONE:
			return LT_AUTH_OPEN;
		case WIFI_SECURITY_WEP:
			return LT_AUTH_WEP;
		case WIFI_SECURITY_WPA_TKIP:
		case WIFI_SECURITY_WPA_AES:
			return LT_AUTH_WPA_PSK;
		case WIFI_SECURITY_WPA_MIXED:
			return LT_AUTH_WPA_WPA2_PSK;
		case WIFI_SECURITY_WPA2_TKIP:
		case WIFI_SECURITY_WPA2_AES:
		case WIFI_SECURITY_WPA2_MIXED:
			return LT_AUTH_WPA2_PSK;
		case WIFI_SECURITY_WPA3_SAE:
			return LT_AUTH_WPA3_PSK;
		case WIFI_SECURITY_WPA3_WPA2_MIXED:
			return LT_AUTH_WPA2_WPA3_PSK;
		case WIFI_SECURITY_EAP:
			return LT_AUTH_WPA2_ENT;
		case WIFI_SECURITY_TYPE_WAPI_PSK:
			return LT_AUTH_WAPI_PSK;
		default:
			return LT_AUTH_INVALID;
	}
}

static uint16_t reason_to_lt(int reason) {
	switch (reason) {
		case WIFI_REASON_BEACON_LOST:
			return LT_REASON_BEACON_TIMEOUT;
		case WIFI_REASON_NO_AP_FOUND:
			return LT_REASON_NO_AP_FOUND;
		case WIFI_REASON_WRONG_PASSWORD:
			return LT_REASON_AUTH_FAIL;
		default:
			// 802.11 reason codes are the same in both APIs
			if (reason > 0 && reason < 200)
				return reason;
			return LT_REASON_UNSPECIFIED;
	}
}

static bk_err_t wifi_event_cb(void *arg, event_module_t module, int event_id, void *event_data) {
	if (!event_cb)
		return BK_OK;
	if (module == EVENT_MOD_WIFI) {
		switch (event_id) {
			case EVENT_WIFI_SCAN_DONE:
				event_cb(LTW_EVENT_SCAN_DONE, 0);
				break;
			case EVENT_WIFI_STA_CONNECTED:
				sta_linked = true;
				event_cb(LTW_EVENT_STA_CONNECTED, 0);
				break;
			case EVENT_WIFI_STA_DISCONNECTED: {
				wifi_event_sta_disconnected_t *info = (wifi_event_sta_disconnected_t *)event_data;
				if (info)
					LT_DM(
						WIFI,
						"STA disconnected, ARMINO reason %d, local %d",
						info->disconnect_reason,
						info->local_generated
					);
				// without an existing link, ARMINO also reports state changes that are no connection failures:
				// - after a scan, wpa_supplicant leaves the scan state with reason 0 (not local),
				//   which does not abort a running connection attempt
				// - stopping the STA interface reports a local 802.11 reason (e.g. 3, DEAUTH_LEAVING)
				// real failures use ARMINO's own reasons (>= 256: NO_AP_FOUND, WRONG_PASSWORD, ...)
				if (!sta_linked && info &&
					(info->local_generated ? info->disconnect_reason < 256 : info->disconnect_reason == 0))
					break;
				sta_linked = false;
				event_cb(LTW_EVENT_STA_DISCONNECTED, info ? reason_to_lt(info->disconnect_reason) : 0);
				break;
			}
			case EVENT_WIFI_AP_CONNECTED:
				event_cb(LTW_EVENT_AP_STA_CONNECTED, 0);
				break;
			case EVENT_WIFI_AP_DISCONNECTED:
				event_cb(LTW_EVENT_AP_STA_DISCONNECTED, 0);
				break;
		}
	} else if (module == EVENT_MOD_NETIF && event_id == EVENT_NETIF_GOT_IP4) {
		netif_event_got_ip4_t *info = (netif_event_got_ip4_t *)event_data;
		if (!info || info->netif_if == NETIF_IF_STA) {
			// ARMINO ignores power save requests while the SoftAP runs; apply it again when connected
			if (sta_ps_enabled)
				bk_wifi_sta_pm_enable();
			event_cb(LTW_EVENT_STA_GOT_IP, 0);
		}
	}
	return BK_OK;
}

void ltw_init(ltw_event_cb_t cb) {
	if (event_cb) {
		event_cb = cb;
		return;
	}
	event_cb = cb;
	bk_event_register_cb(EVENT_MOD_WIFI, EVENT_ID_ALL, wifi_event_cb, NULL);
	bk_event_register_cb(EVENT_MOD_NETIF, EVENT_NETIF_GOT_IP4, wifi_event_cb, NULL);
}

bool ltw_sta_start(void) {
	return bk_wifi_sta_start() == BK_OK;
}

bool ltw_sta_stop(void) {
	return bk_wifi_sta_stop() == BK_OK;
}

bool ltw_sta_connect(const char *ssid, const char *pass, uint8_t channel, const uint8_t *bssid) {
	wifi_sta_config_t config = WIFI_DEFAULT_STA_CONFIG();
	strncpy(config.ssid, ssid, WIFI_SSID_STR_LEN - 1);
	if (pass)
		strncpy(config.password, pass, WIFI_PASSWORD_LEN - 1);
	config.channel = channel;
	if (bssid)
		memcpy(config.bssid, bssid, WIFI_BSSID_LEN);
	if (bk_wifi_sta_set_config(&config) != BK_OK)
		return false;
	return bk_wifi_sta_start() == BK_OK;
}

bool ltw_sta_disconnect(void) {
	return bk_wifi_sta_disconnect() == BK_OK;
}

bool ltw_sta_get_link(ltw_ap_info_t *info) {
	wifi_link_status_t link;
	memset(&link, 0, sizeof(link));
	if (bk_wifi_sta_get_link_status(&link) != BK_OK)
		return false;
	if (link.state != WIFI_LINKSTATE_STA_GOT_IP && link.state != WIFI_LINKSTATE_STA_CONNECTED)
		return false;
	memset(info, 0, sizeof(*info));
	strncpy(info->ssid, link.ssid, sizeof(info->ssid) - 1);
	memcpy(info->bssid, link.bssid, 6);
	info->rssi	  = link.rssi;
	info->channel = link.channel;
	info->auth	  = security_to_auth(link.security);
	return true;
}

static void ip4_to_config(const ltw_ip4_t *ip, netif_ip4_config_t *config) {
	const uint8_t *b;
	memset(config, 0, sizeof(*config));
#define IP4_STR(dst, val)        \
	b = (const uint8_t *)&(val); \
	snprintf(dst, NETIF_IP4_STR_LEN, "%u.%u.%u.%u", b[0], b[1], b[2], b[3]);
	IP4_STR(config->ip, ip->ip);
	IP4_STR(config->mask, ip->mask);
	IP4_STR(config->gateway, ip->gateway);
	IP4_STR(config->dns, ip->dns);
#undef IP4_STR
}

static uint32_t str_to_ip4(const char *str) {
	unsigned a, b, c, d;
	if (sscanf(str, "%u.%u.%u.%u", &a, &b, &c, &d) != 4)
		return 0;
	return (a & 0xFF) | ((b & 0xFF) << 8) | ((c & 0xFF) << 16) | ((d & 0xFF) << 24);
}

bool ltw_sta_set_ip4(const ltw_ip4_t *ip) {
	if (!ip)
		return bk_netif_dhcpc_start(NETIF_IF_STA) == BK_OK;
	netif_ip4_config_t config;
	ip4_to_config(ip, &config);
	return bk_netif_set_ip4_config(NETIF_IF_STA, &config) == BK_OK;
}

bool ltw_ap_set_ip4(const ltw_ip4_t *ip) {
	netif_ip4_config_t config;
	ip4_to_config(ip, &config);
	return bk_netif_set_ip4_config(NETIF_IF_AP, &config) == BK_OK;
}

bool ltw_ap_start(const char *ssid, const char *pass, uint8_t channel, bool hidden, uint8_t max_conn) {
	wifi_ap_config_t config = WIFI_DEFAULT_AP_CONFIG();
	strncpy(config.ssid, ssid, WIFI_SSID_STR_LEN - 1);
	if (pass)
		strncpy(config.password, pass, WIFI_PASSWORD_LEN - 1);
	config.channel = channel;
	config.hidden  = hidden;
	// ARMINO falls back to its default (with a warning) for values above the supported maximum
	if (max_conn > CONFIG_WIFI_MAC_SUPPORT_STAS_MAX_NUM)
		max_conn = CONFIG_WIFI_MAC_SUPPORT_STAS_MAX_NUM;
	config.max_con = max_conn;
	if (bk_wifi_ap_set_config(&config) != BK_OK)
		return false;
	return bk_wifi_ap_start() == BK_OK;
}

bool ltw_ap_stop(void) {
	bool ret = bk_wifi_ap_stop() == BK_OK;
	// power save is only possible without the SoftAP
	if (sta_ps_enabled)
		bk_wifi_sta_pm_enable();
	return ret;
}

uint8_t ltw_ap_get_sta_num(void) {
	wlan_ap_stas_t stas = {0};
	if (bk_wifi_ap_get_sta_list(&stas) != BK_OK)
		stas.num = 0;
	if (stas.sta)
		os_free(stas.sta);
	return stas.num;
}

bool ltw_get_ip4(bool ap, ltw_ip4_t *ip) {
	netif_ip4_config_t config;
	memset(&config, 0, sizeof(config));
	if (bk_netif_get_ip4_config(ap ? NETIF_IF_AP : NETIF_IF_STA, &config) != BK_OK)
		return false;
	ip->ip		= str_to_ip4(config.ip);
	ip->mask	= str_to_ip4(config.mask);
	ip->gateway = str_to_ip4(config.gateway);
	ip->dns		= str_to_ip4(config.dns);
	return ip->ip != 0;
}

bool ltw_get_mac(bool ap, uint8_t *mac) {
	return bk_get_mac(mac, ap ? MAC_TYPE_AP : MAC_TYPE_STA) == BK_OK;
}

bool ltw_set_mac(const uint8_t *mac) {
	return bk_set_base_mac(mac) == BK_OK;
}

void *ltw_get_netif(bool ap) {
	return ap ? net_get_uap_handle() : net_get_sta_handle();
}

bool ltw_set_sleep(bool enable) {
	sta_ps_enabled = enable;
	return (enable ? bk_wifi_sta_pm_enable() : bk_wifi_sta_pm_disable()) == BK_OK;
}

bool ltw_scan_start(void) {
	return bk_wifi_scan_start(NULL) == BK_OK;
}

ltw_ap_info_t *ltw_scan_get_result(int *count) {
	wifi_scan_result_t result = {0};
	*count					  = 0;
	if (bk_wifi_scan_get_result(&result) != BK_OK)
		return NULL;
	ltw_ap_info_t *list = NULL;
	if (result.ap_num > 0)
		list = (ltw_ap_info_t *)calloc(result.ap_num, sizeof(ltw_ap_info_t));
	if (list) {
		for (int i = 0; i < result.ap_num; i++) {
			strncpy(list[i].ssid, result.aps[i].ssid, sizeof(list[i].ssid) - 1);
			memcpy(list[i].bssid, result.aps[i].bssid, 6);
			list[i].rssi	= result.aps[i].rssi;
			list[i].channel = result.aps[i].channel;
			list[i].auth	= security_to_auth(result.aps[i].security);
		}
		*count = result.ap_num;
	}
	bk_wifi_scan_free_result(&result);
	return list;
}
