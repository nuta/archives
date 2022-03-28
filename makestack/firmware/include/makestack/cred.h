#ifndef __MAKESTACK_CRED_H__
#define __MAKESTACK_CRED_H__

#include <makestack/types.h>

struct cred {
    uint64_t version;
    // These ASCII strings are guaranteed null-terminated.
    // wifi_ssid is an empty string if the wifi_adapter is disabled.
    char adapter[8];
    char wifi_ssid[64];
    char wifi_password[64];
    char server_url[256];
} __attribute__((packed));

const extern struct cred __cred;

#endif
