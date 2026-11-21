/* Copyright (c) Kuba Szczodrzyński 2022-06-26. */

#pragma once

#include <ArduinoPrivate.h>
#include <WiFi.h>

// ARMINO's wifi_types.h uses some names of the Arduino WiFi API
#define wifi_err_reason_t					 armino_wifi_err_reason_t
#define wifi_event_sta_connected_t			 armino_wifi_event_sta_connected_t
#define wifi_event_sta_disconnected_t		 armino_wifi_event_sta_disconnected_t
#define WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT	 ARMINO_WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT
#define WIFI_REASON_BSS_TRANSITION_DISASSOC	 ARMINO_WIFI_REASON_BSS_TRANSITION_DISASSOC
#define WIFI_REASON_CIPHER_SUITE_REJECTED	 ARMINO_WIFI_REASON_CIPHER_SUITE_REJECTED
#define WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT ARMINO_WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT
#define WIFI_REASON_IE_IN_4WAY_DIFFERS		 ARMINO_WIFI_REASON_IE_IN_4WAY_DIFFERS
#define WIFI_REASON_INVALID_PMKID			 ARMINO_WIFI_REASON_INVALID_PMKID
#define WIFI_REASON_NO_AP_FOUND				 ARMINO_WIFI_REASON_NO_AP_FOUND
#define WIFI_REASON_UNSPECIFIED				 ARMINO_WIFI_REASON_UNSPECIFIED

extern "C" {

#include <lwip/api.h>
#include <lwip/ip_addr.h>
#include <lwip/netif.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include <task.h>

#include <components/event.h>
#include <components/netif.h>
#include <components/system.h>
#include <modules/pm.h>
#include <modules/wifi.h>
#include <os/mem.h>

extern void *net_get_sta_handle(void);
extern void *net_get_uap_handle(void);

} // extern "C"

#undef wifi_err_reason_t
#undef wifi_event_sta_connected_t
#undef wifi_event_sta_disconnected_t
#undef WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT
#undef WIFI_REASON_BSS_TRANSITION_DISASSOC
#undef WIFI_REASON_CIPHER_SUITE_REJECTED
#undef WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT
#undef WIFI_REASON_IE_IN_4WAY_DIFFERS
#undef WIFI_REASON_INVALID_PMKID
#undef WIFI_REASON_NO_AP_FOUND
#undef WIFI_REASON_UNSPECIFIED

// WiFi.cpp
WiFiAuthMode securityToAuthMode(wifi_security_t security);
void ipToConfig(netif_ip4_config_t *config, IPAddress ip, IPAddress mask, IPAddress gateway, IPAddress dns);
IPAddress configToIp(const char *str);

// WiFiEvents.cpp
extern void wifiEventSendArduino(EventId event);
extern void startWifiTask();

// WiFiGeneric.cpp
extern void wifiUpdateSleep();

#ifndef MACSTR
#define MACSTR	   "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC2STR(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#endif

typedef struct {
	SemaphoreHandle_t scanSem;
	WiFiStatus staStatus;
	bool staEnabled;
	bool apEnabled;
	bool apRunning;
	bool staLinked;
	bool staLost;
	bool staHasIp;
	bool sleep;
	bool useStaticIp;
	netif_ip4_config_t staIp;
	netif_ip4_config_t apIp;
	char ssid[WIFI_SSID_STR_LEN];
	char pass[WIFI_PASSWORD_LEN];
	char apSsid[WIFI_SSID_STR_LEN];
	int32_t channel;
	wifi_link_status_t link;
} WiFiData;

#define DATA  ((WiFiData *)data)
#define pDATA ((WiFiData *)pWiFi->data)
