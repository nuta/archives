---
title: Simple Messaging/Managing System
---

Simple Messaging/Managing System (SMMS in short) is a communication protocol for MakeStack.

Format
------
A payload of SMMS is an array of simple *Type-Length-Value* messages.

```c
struct Payload {
    header
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
 0x03  | device_id
 0x04  | log
 0x05  | command (key length, key, and config value)
 0x07  | report (type and value)
 0x09  | config (key length, key, and config value)
 0x0a  | update (method, version)
