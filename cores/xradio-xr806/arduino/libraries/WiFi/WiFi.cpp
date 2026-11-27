#include "WiFiPrivate.h"

WiFiClass::WiFiClass() {
	data		  = (WiFiData *)calloc(1, sizeof(WiFiData));
	DATA->scanSem = xSemaphoreCreateBinary();
}

WiFiClass::~WiFiClass() {
	vSemaphoreDelete(DATA->scanSem);
	free(data);
	data = NULL;
}

WiFiAuthMode wpaFlagsToAuthMode(int flags) {
	if ((flags & WPA_FLAGS_WPA) && (flags & WPA_FLAGS_WPA2))
		return WIFI_AUTH_WPA_WPA2_PSK;
	if (flags & WPA_FLAGS_WPA2)
		return WIFI_AUTH_WPA2_PSK;
	if (flags & WPA_FLAGS_WPA)
		return WIFI_AUTH_WPA_PSK;
	if (flags & WPA_FLAGS_WEP)
		return WIFI_AUTH_WEP;
	return WIFI_AUTH_OPEN;
}
