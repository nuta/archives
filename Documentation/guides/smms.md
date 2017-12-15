---
title: Simple Messaging/Managing System
---

Simple Messaging/Managing System (SMMS in short) is a communication protocol for MakeStack.

Format
------
A payload of SMMS is an array of simple *Type-Length-Value* messages.

```c
struct Payload {
    Message Messages[n];
};

struct Message {
    uint8_t typeId;

    /*
     *  The length of `length' field is variable depends on the length of value. The encoding
     *  scheme is the same as MQTT. Refer "Remaining Length" section in the MQTT spec:
     *
     *    https://public.dhe.ibm.com/software/dw/webservices/ws-mqtt/mqtt-v3r1.html
     *
     */
    uint8_t length[m];
    uint8_t value[length];
};
```

Messages
--------

| Type ID | name |
:-----:|:-----:
 0x00  | (reserved)
 0x01  | version (must be `1`)
 0x02  | qos (unimplemented yet)
 0x03  | qos_reply (unimplemented yet)
 0x04  | nonce (unused)
 0x06  | hmac (HMAC-SHA2)
 0x07  | timestamp (ISO 8601)
 0x0a  | device_id
 0x0b  | device_info (0-2 bits: `state`, 3: `debug mode`, 4-5: `os type`)
 0x0c  | log
 0x0d  | crashdump (unimplemented yet)
 0x10  | os_version
 0x11  | app_version
 0x12  | os_image_hmac
 0x13  | app_image_hmac
 0x20  | config (`key length`, `key`, and `config value`)
