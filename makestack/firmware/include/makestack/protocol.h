#ifndef __MAKESTACK_PROTOCOL_H__
#define __MAKESTACK_PROTOCOL_H__

#define PONG_DATA "Sup!"
#define PONG_DATA_LEN  4
#define FIRMWARE_DATA_RAW     0x01
#define FIRMWARE_DATA_DEFLATE 0x02
#define FIRMWARE_DATA_EOF     0xff

struct payload_header {
    uint16_t len;
    uint16_t checksum;
} __attribute__((packed));

struct firmware_data_header {
    uint8_t type;
    uint32_t offset;
} __attribute__((packed));

struct firmware_request {
    uint64_t version;
    uint32_t offset;
} __attribute__((packed));

struct device_status {
    uint8_t state;
    uint8_t battery_level;
    uint16_t reserved;
    uint32_t ram_free;
} __attribute__((packed));

void process_payload(uint8_t *payload, size_t payload_len);
size_t build_payload(uint8_t *buf, size_t buf_len);
void adapter_read_error();

#endif
