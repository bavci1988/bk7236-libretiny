#pragma once

// Thin C layer over the ARMINO Wi-Fi/netif/event APIs.
// The ARMINO headers define WIFI_REASON_* enumerators, which collide with
// the LibreTiny WiFi API, so they are only included by wifi_armino.c.

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	LTW_EVENT_SCAN_DONE = 0,
	LTW_EVENT_STA_CONNECTED,
	LTW_EVENT_STA_DISCONNECTED,
	LTW_EVENT_STA_GOT_IP,
	LTW_EVENT_AP_STA_CONNECTED,
	LTW_EVENT_AP_STA_DISCONNECTED,
} ltw_event_t;

typedef struct {
	char ssid[33];
	uint8_t bssid[6];
	int rssi;
	uint8_t channel;
	uint8_t auth; // WiFiAuthMode
} ltw_ap_info_t;

typedef struct {
	uint32_t ip;
	uint32_t mask;
	uint32_t gateway;
	uint32_t dns;
} ltw_ip4_t;

// reason is a LibreTiny/ESP wifi_err_reason_t value
typedef void (*ltw_event_cb_t)(ltw_event_t event, uint16_t reason);

void ltw_init(ltw_event_cb_t cb);

bool ltw_sta_start(void);
bool ltw_sta_stop(void);
bool ltw_sta_connect(const char *ssid, const char *pass, uint8_t channel, const uint8_t *bssid);
bool ltw_sta_disconnect(void);
bool ltw_sta_get_link(ltw_ap_info_t *info);
bool ltw_sta_set_ip4(const ltw_ip4_t *ip); // NULL: use DHCP

bool ltw_ap_start(const char *ssid, const char *pass, uint8_t channel, bool hidden, uint8_t max_conn);
bool ltw_ap_stop(void);
uint8_t ltw_ap_get_sta_num(void);
bool ltw_ap_set_ip4(const ltw_ip4_t *ip);

bool ltw_get_ip4(bool ap, ltw_ip4_t *ip);
bool ltw_get_mac(bool ap, uint8_t *mac);
bool ltw_set_mac(const uint8_t *mac);
void *ltw_get_netif(bool ap);

bool ltw_set_sleep(bool enable);

bool ltw_scan_start(void);
// returns a malloc()-ed array, to be freed by the caller
ltw_ap_info_t *ltw_scan_get_result(int *count);

#ifdef __cplusplus
} // extern "C"
#endif
