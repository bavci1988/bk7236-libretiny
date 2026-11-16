#pragma once

#include_next "lwipopts.h"

// mDNS support
#undef LWIP_NUM_NETIF_CLIENT_DATA
#define LWIP_NUM_NETIF_CLIENT_DATA	   2
#define LWIP_NETIF_EXT_STATUS_CALLBACK 1

// increase TCP/IP thread stack size (was 512 words) - mDNS responder overflows it
#undef TCPIP_THREAD_STACKSIZE
#define TCPIP_THREAD_STACKSIZE 1024
