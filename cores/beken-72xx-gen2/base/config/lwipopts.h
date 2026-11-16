#pragma once

#include_next "lwipopts.h"

// mDNS support
#undef LWIP_NUM_NETIF_CLIENT_DATA
#define LWIP_NUM_NETIF_CLIENT_DATA	   2
#define LWIP_NETIF_EXT_STATUS_CALLBACK 1
