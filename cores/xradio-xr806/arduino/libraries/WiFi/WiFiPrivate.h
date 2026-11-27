#pragma once

#include <ArduinoPrivate.h>
#include <WiFi.h>

extern "C" {
// copy defines from PIO builder (for IDE to understand)
#define LWIP_TIMEVAL_PRIVATE 0
#define LWIP_NETIF_HOSTNAME	 1
#define LWIP_SO_RCVBUF		 1

#include <lwip/api.h>
#include <lwip/dns.h>
#include <lwip/ip_addr.h>
#include <lwip/netif.h>

#include <sdk_private.h>

#include "common/framework/net_ctrl.h"
#include "common/framework/sys_ctrl/sys_ctrl.h"
#include "common/framework/sysinfo.h"
#include "net/udhcp/usr_dhcpd.h"
#include "net/wlan/wlan.h"
#include "net/wlan/wlan_ext_req.h"

#include <FreeRTOS.h>
#include <semphr.h>

// base/api/xr806_pm.c
bool xr806_wlan_apply_power_save(struct netif *nif, bool enable);

} // extern "C"

// WiFi.cpp
extern WiFiAuthMode wpaFlagsToAuthMode(int flags);

// WiFiEvents.cpp
extern void wifiEventSendArduino(EventId event);
extern bool registerWifiHandlers();

// WiFiScan.cpp
extern void wifiScanDone(bool success);

// The radio runs either the station or the access point (one netif).
typedef struct {
	bool initialized;
	bool sleep;
	bool staEnabled;
	bool apEnabled;
	WiFiMode mode;
	SemaphoreHandle_t scanSem;
	WiFiNetworkInfo sta;
	WiFiNetworkInfo ap;
} WiFiData;

#define DATA  ((WiFiData *)data)
#define pDATA ((WiFiData *)pWiFi->data)
#define cDATA ((WiFiData *)cls->data)

static inline struct netif *staNetif() {
	return wlan_netif_get(WLAN_MODE_STA);
}

static inline struct netif *apNetif() {
	return wlan_netif_get(WLAN_MODE_HOSTAP);
}
