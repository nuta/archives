#include "telemata.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory>
#include <string>
#include "logger.h"
#include "utils.h"

#define TELEMATA_VERSION 1
#define TELEMATA_HMAC_MSG 0x01
#define TELEMATA_CACHE_MSG 0x02
#define TELEMATA_DEVICE_ID_MSG 0x03
#define TELEMATA_LOG_MSG 0x04
#define TELEMATA_COMMAND_MSG 0x05
#define TELEMATA_GET_MSG 0x06
#define TELEMATA_OBSERVE_MSG 0x07
#define TELEMATA_REPORT_MSG 0x08
#define TELEMATA_CONFIG_MSG 0x09
#define TELEMATA_UPDATE_MSG 0x0a
#define TELEMATA_OSUPDATE_MSG 0x0b
#define TELEMATA_CURRENT_VERSION_REPORT 0x0001

using namespace std;

class TelemataPayloadBuilder {
   private:
    uint8_t *ptr;
    int allocated_length;
    int remaining;

   public:
    void *payload;

    TelemataPayloadBuilder() {
        remaining = allocated_length = 4096;
        payload = malloc(remaining);
        ptr = (uint8_t *) payload;
    }

    ~TelemataPayloadBuilder() {
        free(payload);
    }

    int append(int type, void *buf, int length) {
        int i = fill_variable_length(ptr + 1, remaining - 1, length);
        if (remaining - (1 + i + length) < 0) {
            return -1;
        }

        *ptr++ = type;
        memcpy(ptr + i, buf, length);
        ptr += i + length;
        remaining -= 1 + i + length;
        return 0;
    }

    size_t length() {
        return allocated_length - remaining;
    }

    static int fill_variable_length(uint8_t *buf, int buf_length, int length) {
        int i = 0;
        for (; i < buf_length && i < length; i++) {
            int digit = length % 0x80;
            length = length / 0x80;
            buf[i] = (uint8_t)((length > 0) ? 0x80 : 0) | digit;
        }

        return i;
    }
};

TelemataClient::TelemataClient(const char *device_id)
    : log_length(0),
      log_index(0),
      config(),
      current_app_version(0),
      device_id(device_id) {
    log_allocated_length = 2048;
    log_buffer = (char *) malloc(log_allocated_length);
}

void TelemataClient::append_log(char ch) {
    log_buffer[log_index] = ch;
    log_length += (log_length == log_allocated_length) ? 0 : 1;
    log_index++;
    if (log_index == log_allocated_length) {
        log_index = 0;
    }
}

void TelemataClient::append_log(const char *str) {
    while (*str != '\0') {
        append_log(*str);
        str++;
    }
}

int parse_variable_length(uint8_t *buf, int buf_length, int *length) {
    *length = 0;
    int base = 1;
    for (int i = 0; i < buf_length; i++) {
        uint8_t byte = buf[i];
        *length += (byte & 0x7f) * base;

        if ((byte & 0x80) == 0) {
            return i + 1;
        }

        base *= 128;
    }

    // Invalid format.
    return -1;
}

#define PARSE_KEY_VALUE_MSG(len, key, value) \
    int key_len;                                                           \
    int key_len_len = parse_variable_length(p, remaining, &key_len);       \
    int value_len = len - key_len_len - key_len;                           \
    if (value_len < 0 || key_len_len + key_len + value_len > remaining) {  \
       printf("inval cmd: %d %d %d\n", value_len, key_len, remaining); \
        goto next_message;                                                 \
    }                                                                      \
                                                                           \
    char *key = (char *) malloc(key_len + 1);                              \
    memcpy(key, p + key_len_len, key_len);                                 \
    key[key_len] = '\0';                                                   \
    char *value = (char *) malloc(value_len + 1);                          \
    memcpy(value, p + key_len_len + key_len, value_len);                   \
    value[value_len] = '\0';

// Returns 0 on success, or -1 on failure, or 1 if the server is sending
// a new firmware.
int TelemataClient::receive_payload(const void *payload, size_t payload_length) {
    printf("payload(%d): ", payload_length);
    for (int i = 0; i < 20; i++)
        printf("%x ", *((char *) payload + i));
    printf("\n");

    if (payload_length < 2) {
        // Ignore a malformed payload.
        return -1;
    }


    uint8_t *p = (uint8_t *) payload;

    if (*p >> 4 != 1) {
        printf("telemata: unsupported version, ignoring...\n");
        return -1;
    }
    p++;

    // Skip the payload header.
    int len;
    int len_len;
    if ((len_len = parse_variable_length(p, payload_length - 1, &len)) < 0) {
        return -1;
    }

    p += len_len;

    int remaining = payload_length;
    while (remaining > 2) {
        int type = *p;
        p++;
        remaining--;

        if ((len_len = parse_variable_length(p, remaining, &len)) < 0) {
            return -1;
        }

        p += len_len;
        remaining -= len_len;

        switch (type) {
            case TELEMATA_CONFIG_MSG: {
                PARSE_KEY_VALUE_MSG(len, key, value);
                // update_config(key, value);
                free(key);
                free(value);
                break;
            }
            case TELEMATA_COMMAND_MSG: {
                PARSE_KEY_VALUE_MSG(len, key, value);
                // execute_command(key, value);
                printf("command: %s(\"%s\")\n", key, value);
                free(key);
                free(value);
                break;
            }
            case TELEMATA_UPDATE_MSG: {
                if (len < 5) {
                    goto next_message;
                }

                int version = from_be32(p + 1);
                printf("APP: %d %d\n", APP_VERSION, version);

#ifdef DEBUG_BUILD
                if (APP_VERSION != version) {
#else
                if (APP_VERSION > version) {
#endif
                    INFO("Time to update!\n");
                    download_app(version);
                }

                return 1;
            }
        }

next_message:
        printf("next: %d %d\n", len, remaining);
        p += len;
        remaining -= len;
    }

    return 0;
}

void TelemataClient::send() {
    int include_device_id = 1;

    TelemataPayloadBuilder payload;

    if (include_device_id) {
        payload.append(TELEMATA_DEVICE_ID_MSG, (void *) device_id,
                       strlen((char *) device_id));
    }

    if (log_length > 0) {
        payload.append(TELEMATA_LOG_MSG, log_buffer, log_length);
        log_index = 0;
        log_length = 0;
    }

    uint8_t ver_report_msg[6];
    to_be16(&ver_report_msg[0], TELEMATA_CURRENT_VERSION_REPORT);
    to_be32(&ver_report_msg[2], current_app_version);
    payload.append(TELEMATA_REPORT_MSG, ver_report_msg, sizeof(ver_report_msg));

    // Construct the header.
    uint8_t *ptr = (uint8_t *) malloc(1 + 4 + payload.length());
    ptr[0] = TELEMATA_VERSION << 4;
    int len_len =
        TelemataPayloadBuilder::fill_variable_length(ptr + 1, 4, payload.length());
    memcpy(ptr + 1 + len_len, payload.payload, payload.length());

    send_payload((void *) ptr, 1 + len_len + payload.length());
    free(ptr);
}
