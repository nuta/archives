#include <esp_system.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory>
#include <string>
#include "telemata.h"
#include "logger.h"
#include "utils.h"

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
        for (; i < buf_length && length > 0; i++) {
            int digit = length % 0x80;
            length = length / 0x80;
            buf[i] = (uint8_t)((length > 0) ? 0x80 : 0) | digit;
        }

        return i;
    }
};

TelemataClient::TelemataClient(const char *device_name)
    : log_length(0),
      log_index(0),
      config(),
      first_send(true),
      device_name(device_name) {
    log_allocated_length = 2048;
    log_buffer = (char *) malloc(log_allocated_length);
    command_callback = nullptr;
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

#define PARSE_KEY_VALUE_MSG(len) \
    int key_len;                                                               \
    int key_len_len = parse_variable_length(p + 1, remaining - 1, &key_len);   \
    int value_len = len - key_len_len - key_len - 1;                           \
    if (value_len < 0 || 1 + key_len_len + key_len + value_len > remaining) {  \
       printf("invalid kv: %d %d %d\n", value_len, key_len, remaining);        \
        goto next_message;                                                     \
    }                                                                          \
                                                                               \
    int value_type = p[0];                                                     \
    char *key = (char *) malloc(key_len + 1);                                  \
    memcpy(key, p + 1 + key_len_len, key_len);                                 \
    key[key_len] = '\0';                                                       \
    char *value = (char *) malloc(value_len + 1);                              \
    memcpy(value, p + 1 + key_len_len + key_len, value_len);                   \
    value[value_len] = '\0';

// Returns 0 on success, or -1 on failure, or 1 if the server is sending
// a new firmware.
int TelemataClient::receive_payload(const void *payload, size_t payload_length) {
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
            case TELEMATA_COMMAND_MSG: {
                PARSE_KEY_VALUE_MSG(len);
                // execute_command(key, value);
                if (value_type == 0x01 /* string */ && command_callback) {
                    printf("command: %s(\"%s\")\n", key, value);
                    command_callback(key, value);
                }

                free(key);
                free(value);
                break;
            }
            case TELEMATA_UPDATE_MSG: {
                if (len < 5) {
                    goto next_message;
                }

                int version = from_be32(p + 1);

#ifdef DEBUG_BUILD
                if (APP_VERSION != version) {
#else
                if (APP_VERSION > version) {
#endif
                    INFO("Time to update! (%d -> %d)\n", APP_VERSION, version);
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
    TelemataPayloadBuilder payload;

    payload.append(TELEMATA_DEVICE_NAME_MSG, (void *) device_name,
                   strlen((char *) device_name));

    struct device_state_msg device_state;
    device_state.state = first_send ? DEVICE_BOOTED : DEVICE_RUNNING;
    device_state.type = DEVICE_TYPE_ESP32;
    device_state.version = APP_VERSION;
    device_state.ram_free = esp_get_free_heap_size();
    device_state.battery = 0xf0; /* Unsupported */
    payload.append(TELEMATA_DEVICE_STATE_MSG, &device_state, sizeof(device_state));

    if (log_length > 0) {
        payload.append(TELEMATA_LOG_MSG, log_buffer, log_length);
        log_index = 0;
        log_length = 0;
    }
    // Construct the header.
    uint8_t *ptr = (uint8_t *) malloc(1 + 4 + payload.length());
    ptr[0] = TELEMATA_VERSION << 4;
    int len_len =
        TelemataPayloadBuilder::fill_variable_length(ptr + 1, 4, payload.length());
    memcpy(ptr + 1 + len_len, payload.payload, payload.length());

    send_payload((void *) ptr, 1 + len_len + payload.length());
    free(ptr);

    first_send = false;
}

void TelemataClient::set_command_callback(void (*callback)(const char *name, const char *arg)) {
    command_callback = callback;
}
