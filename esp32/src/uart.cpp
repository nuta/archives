#include <string.h>
#include <driver/uart.h>
#include "uart.h"
#include <esp_system.h>
#include <esp_ota_ops.h>

#define BUF_SIZE 4096
#define READ_BUF_SIZE 1024
#define PACKET_HEADER_LEN 3
#define TXD  (GPIO_NUM_1)
#define RXD  (GPIO_NUM_3)
#define RTS  (GPIO_NUM_22)
#define CTS  (GPIO_NUM_19)

UartTelemataClient::UartTelemataClient()
    : TelemataClient("")  {

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_CTS_RTS,
        .rx_flow_ctrl_thresh = 122,
        .use_ref_tick = 0
    };
    uart_param_config(UART_NUM_0, &uart_config);
    uart_set_pin(UART_NUM_0, TXD, RXD, RTS, CTS);
    uart_driver_install(UART_NUM_0, 16 * 1024, 0, 0, NULL, 0);
}

struct firmware_header {
    uint8_t magic[4];
    uint32_t length;
    uint32_t version;
    uint32_t reserved;
} __attribute__((packed));

void download_firmware() {
    int buf_size = 16 * 1024;

    // Allocate first; malloc would take too long.
    uint8_t *buf = (uint8_t *) malloc(buf_size);

    esp_ota_handle_t update_handle;
    const esp_partition_t *part;
    part = esp_ota_get_next_update_partition(NULL);
    if (part == NULL) {
        printf("%s: failed to get next partition\n", __func__);
        free(buf);
        return;
    }

    esp_err_t err = esp_ota_begin(part, OTA_SIZE_UNKNOWN, &update_handle);
    if (err != ESP_OK) {
        printf("esp_ota_begin: %s\n", esp_err_to_name(err));
        free(buf);
        return;
    }

    // Read the firmware header.
    struct firmware_header header;
    int len = uart_read_bytes(UART_NUM_0, (uint8_t *) &header,
                              sizeof(header), 20 / portTICK_RATE_MS);
    if (len < 0) {
        free(buf);
        return;
    }

    if (header.magic[0] != 0x81 || header.magic[1] != 0xf1
        || header.magic[2] != 0x5c || header.magic[3] != 0x5e) {
        printf("invalid firmware signature\n");
        free(buf);
        return;
    }

    int offset = 0;
    while (offset < header.length) {
        printf("free ram: %dKB\n", heap_caps_get_free_size(MALLOC_CAP_DEFAULT) / 1024);
        printf("%d/%d (%d)\n", offset, header.length,  (header.length - offset < buf_size) ? header.length - offset : buf_size);
        // printf("downloading %d/%d KB\n", offset / 1024, header.length / 1024);
        int len = uart_read_bytes(
            UART_NUM_0,
            buf,
            (header.length - offset < buf_size) ? header.length - offset : buf_size,
            20 / portTICK_RATE_MS
        );

        if (len < 0) {
            free(buf);
            return;
        }

        if ((err = esp_ota_write(update_handle, buf, len)) != ESP_OK) {
            printf("esp_ota_write: %s\n", esp_err_to_name(err));
            free(buf);
            return;
        }

        offset += len;
    }

    if (esp_ota_end(update_handle) != ESP_OK) {
        printf("esp_ota_end: %s\n", esp_err_to_name(err));
        free(buf);
        return;
    }

    if ((err = esp_ota_set_boot_partition(part)) != ESP_OK) {
        printf("esp_ota_set_boot_partition: %s\n", esp_err_to_name(err));
        free(buf);
        return;
    }

    printf("restarting the system...\n");
    esp_restart();
    printf("failed to restart :(\n");
}

void UartTelemataClient::poll_uart() {
    uint8_t *payload = (uint8_t *) malloc(BUF_SIZE);
    uint8_t *new_data = (uint8_t *) malloc(READ_BUF_SIZE);

    while (1) {
retry:
        int len = uart_read_bytes(UART_NUM_0, new_data, 1, 100 / portTICK_RATE_MS);
        if (len < 0 || new_data[0] != 0xaa) {
            goto retry;
        }

        len = uart_read_bytes(UART_NUM_0, &new_data[1], 2, 20 / portTICK_RATE_MS);
        if (len < 0) {
            goto retry;
        }

        if (new_data[1] != 0xab || new_data[2] != 0xff) {
            // Invalid packet signature, ignoring.
            goto retry;
        }


        printf("reading header\n");
        len = uart_read_bytes(UART_NUM_0, payload, 2, 20 / portTICK_RATE_MS);
        if (len < 2) {
            goto retry;
        }

        // Read payload header.
        int payload_offset = 2;
        int payload_length;
        int lenlen;
        for (int n = 1; n < 5; n++) {
            lenlen = parse_variable_length(&payload[1], payload_offset - 1, &payload_length);
            printf("read: %d %d %d\n", payload_offset, payload_offset, lenlen);
            if (lenlen > 0) {
                break;
            }

            if (BUF_SIZE - payload_offset <= 0) {
                printf("serial_adapter: too long payload, ignoring...\n");
                goto retry;
            }

            len = uart_read_bytes(UART_NUM_0, &payload[payload_offset], 1, 20 / portTICK_RATE_MS);
            if (len < 0) {
                goto retry;
            }

            payload_offset += len;
        }

        // Read payload.
        printf("read payload: %d %d\n", payload_offset, payload_length);
        while (payload_offset < payload_length + 1 + lenlen) {
            if (BUF_SIZE - payload_offset <= 0) {
                printf("serial_adapter: too long payload, ignoring...\n");
                goto retry;
            }

            len = uart_read_bytes(
                UART_NUM_0,
                &payload[payload_offset],
                payload_length + 1 + lenlen - payload_offset,
                20 / portTICK_RATE_MS
            );

            if (len < 0) {
                goto retry;
            }

            payload_offset += len;
        }

        printf("received whole packet: %d rm=%d\n", payload_length, payload_offset);
        if (receive_payload(payload, payload_length + 2) == 1) {
            // Firmware update.
            // TODO: fix download_firmware()
            // download_firmware();
        }

        printf("received: done\n");
    }
}

void UartTelemataClient::send_payload(const void *payload, size_t length) {
    uart_write_bytes(UART_NUM_0, "\xaa\xab\xff", 3);
    uart_write_bytes(UART_NUM_0, (const char *) payload, length);
}

void UartTelemataClient::download_app(int version) {

}

