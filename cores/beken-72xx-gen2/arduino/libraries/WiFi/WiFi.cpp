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
