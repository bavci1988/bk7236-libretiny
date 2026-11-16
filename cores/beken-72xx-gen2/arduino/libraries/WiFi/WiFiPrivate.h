#pragma once

#include <ArduinoPrivate.h>
#include <WiFi.h>

#include "wifi_armino.h"

extern "C" {
#include <lwip/api.h>
#include <lwip/ip_addr.h>
#include <lwip/netif.h>

#include <FreeRTOS.h>
#include <queue.h>
#include <semphr.h>
#include <task.h>
}

#ifndef MACSTR
#define MACSTR		"%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC2STR(a)	(a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#endif

typedef struct {
	SemaphoreHandle_t scanSem;
	WiFiStatus staStatus;
	bool staEnabled;
	bool apEnabled;
	bool sleep;
	bool useStaticIp;
	ltw_ip4_t staticIp;
	ltw_ip4_t apIp;
	char ssid[33];
	char pass[65];
	char apSsid[33];
	uint8_t bssid[6];
	ltw_ap_info_t link;
} WiFiData;

#define DATA  ((WiFiData *)data)
#define pDATA ((WiFiData *)pWiFi->data)

// WiFiEvents.cpp
extern void startWifiTask();
extern void wifiEventSendArduino(EventId event);
