#include "smms.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory>
#include <string>
#include "engine.h"
#include "utils.h"

#define SMMS_VERSION 1
#define SMMS_HMAC_MSG 0x01
#define SMMS_CACHE_MSG 0x02
#define SMMS_DEVICE_ID_MSG 0x03
#define SMMS_LOG_MSG 0x04
#define SMMS_COMMAND_MSG 0x05
#define SMMS_GET_MSG 0x06
#define SMMS_OBSERVE_MSG 0x07
#define SMMS_REPORT_MSG 0x08
#define SMMS_CONFIG_MSG 0x09
#define SMMS_UPDATE_MSG 0x0a
#define SMMS_OSUPDATE_MSG 0x0b
#define SMMS_CURRENT_VERSION_REPORT 0x0001

using namespace std;

class SmmsPayloadBuilder {
   private:
    uint8_t *ptr;
    int allocated_length;
    int remaining;

   public:
    void *payload;

    SmmsPayloadBuilder() {
        remaining = allocated_length = 4096;
        payload = malloc(remaining);
        ptr = (uint8_t *) payload;
    }

    ~SmmsPayloadBuilder() {
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

SmmsClient::SmmsClient(Engine *engine, const char *device_id)
    : log_length(0),
      log_index(0),
      config(),
      current_app_version(0),
      engine(engine),
      device_id(device_id) {
    log_allocated_length = 2048;
    log_buffer = (char *) malloc(log_allocated_length);
}

void SmmsClient::append_log(char ch) {
    log_buffer[log_index] = ch;
    log_length += (log_length == log_allocated_length) ? 0 : 1;
    log_index++;
    if (log_index == log_allocated_length) {
        log_index = 0;
    }
}

void SmmsClient::append_log(const char *str) {
    while (*str != '\0') {
        append_log(*str);
        str++;
    }
}

static int parse_variable_length(uint8_t *buf, int buf_length, int *length) {
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

#define PARSE_KEY_VALUE_MSG(key, value) \
    int key_len;                                                           \
    int key_len_len = parse_variable_length(p, remaining, &key_len);       \
    int value_len = len - key_len;                                         \
    if (key_len_len + key_len + value_len > remaining) {                   \
        goto next_message;                                                 \
    }                                                                      \
                                                                           \
    char *key = (char *) malloc(key_len + 1);                              \
    memcpy(key, p + key_len_len, key_len);                                 \
    key[len] = '\0';                                                       \
                                                                           \
    char *value = (char *) malloc(len + 1);                                \
    memcpy(value, p + key_len_len + key_len, len - key_len_len - key_len); \
    value[len] = '\0';


void SmmsClient::receive_payload(const void *payload, size_t payload_length) {
    hexdump((uint8_t*) payload, payload_length);

    if (payload_length < 2) {
        // Ignore a malformed payload.
        return;
    }

    uint8_t *p = (uint8_t *)payload;
    int remaining = payload_length;

    // Skip the payload header.
    int len;
    int len_len;
    if ((len_len = parse_variable_length(p, remaining, &len)) < 0) {
        return;
    }

    p += 1 + len_len;
    remaining -= 1 + len_len;

    while (remaining > 2) {
        if ((len_len = parse_variable_length(p, remaining, &len)) < 0) {
            return;
        }

        int type = *p;
        p += 1 + len_len;
        remaining -= 1 + len_len;

        switch (type) {
            case SMMS_CONFIG_MSG: {
                PARSE_KEY_VALUE_MSG(key, value);
                engine->update_config(key, value);
                free(key);
                free(value);
                break;
            }
            case SMMS_COMMAND_MSG: {
                PARSE_KEY_VALUE_MSG(key, value);
                engine->execute_command(key, value);
                free(key);
                free(value);
                break;
            }
            case SMMS_UPDATE_MSG: {
                if (len < 5) {
                    goto next_message;
                }

                hexdump(p, 16);
                printf("ver: %d\n", from_be32(p + 1));
                download_app(from_be32(p + 1));
                break;
            }
        }

    next_message:
        p += len;
        remaining -= len;
    }
}

void SmmsClient::send() {
    int include_device_id = 1;

    SmmsPayloadBuilder payload;

    if (include_device_id) {
        payload.append(SMMS_DEVICE_ID_MSG, (void *) device_id,
                       strlen((char *) device_id));
    }

    payload.append(SMMS_LOG_MSG, log_buffer, log_length);
    log_index = 0;
    log_length = 0;

    uint8_t ver_report_msg[6];
    to_be16(&ver_report_msg[0], SMMS_CURRENT_VERSION_REPORT);
    to_be32(&ver_report_msg[2], current_app_version);
    payload.append(SMMS_REPORT_MSG, ver_report_msg, sizeof(ver_report_msg));

    // Construct the header.
    uint8_t *ptr = (uint8_t *) malloc(1 + 4 + payload.length());
    ptr[0] = SMMS_VERSION << 4;
    int len_len =
        SmmsPayloadBuilder::fill_variable_length(ptr + 1, 4, payload.length());
    memcpy(ptr + 1 + len_len, payload.payload, payload.length());

    send_payload((void *) ptr, 1 + len_len + payload.length());
    free(ptr);
}

void SmmsClient::launch_app(const void *image, size_t image_length) {
    const char *script = (const char *)image;

    if (image_length < 2) {
        printf("bad app image\n");
        return;
    }

    if (script[0] == 'P' && script[1] == 'K') {
        printf("ziped app image is not supported\n");
        return;
    }

    printf("launch_app: %s\n", (char *)image);
    engine->run(script, image_length);
}
