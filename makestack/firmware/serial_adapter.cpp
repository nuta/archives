#include <makestack/types.h>
#include <makestack/logger.h>
#include <makestack/protocol.h>
#include <makestack/serial_adapter.h>

#define BUF_LEN (1024 * 9)
#define PACKET_MAGIC "\xe7MPKT"
#define PACKET_MAGIC_LEN 5
#define UART_PORT    UART_NUM_0
#define UART_TX_PIN  GPIO_NUM_1
#define UART_RX_PIN  GPIO_NUM_3

static size_t escape_payload(uint8_t *buf, size_t buf_len, size_t payload_len) {
    size_t i = 0;
    int shifts = 0;
    while (i < payload_len + shifts) {
        uint8_t byte = buf[i];
        if (byte == 0x0a || byte == 0xee) {
            if (buf_len < payload_len + shifts + 1) {
                WARN("too short payload buf");
                return 0;
            }

            memmove(buf + i + 2, buf + i + 1, payload_len - (i - shifts));
            buf[i++] = 0xee;
            buf[i++] = (byte == 0x0a) ? 0xe0 : 0xe1;
            shifts++;
        } else {
            i++;
        }
    }

    return i;
}

static size_t build_packet(uint8_t *buf, size_t buf_len) {
    uint8_t *p = buf;
    *p++ = 0x0a;
    memcpy(p, PACKET_MAGIC, PACKET_MAGIC_LEN);
    p+= PACKET_MAGIC_LEN;

    size_t payload_max_len = buf_len - (1 + PACKET_MAGIC_LEN);
    size_t payload_len;
    size_t escaped_payload_len;

    if ((payload_len = build_payload(p, payload_max_len)) == 0) {
        return 0;
    }

    if ((escaped_payload_len = escape_payload(p, payload_max_len, payload_len)) == 0) {
        return 0;
    }

    p += escaped_payload_len;
    *p++ = 0x0a;

    size_t pkt_len = 1 + PACKET_MAGIC_LEN + escaped_payload_len + 1;
    return pkt_len;
}

static uint8_t *read_packet(uint8_t *buf, size_t buf_len, size_t *payload_len) {
    int read_len = uart_read_bytes(UART_PORT, buf, buf_len, 100 / portTICK_RATE_MS);
    if (read_len < 0) {
        // TODO: Sleep for a while or use interrupt instead of polling.
        return NULL;
    }

    // Look for the packet delimiter (0x0a).
    uint8_t *start, *end;
    if ((start = (uint8_t *) memchr(buf, 0x0a, read_len)) == NULL)
        return NULL;
    size_t remaining_len = read_len - ((size_t) buf - (size_t) start) - 1;
    if ((end = (uint8_t *) memchr(start + 1, 0x0a, remaining_len)) == NULL)
        return NULL;

    // Verify the magic.
    size_t packet_len = (size_t) end - (size_t) start - 1;
    if (packet_len < PACKET_MAGIC_LEN) {
        DEBUG("too short packet");
        return NULL;
    }

    if (memcmp(start + 1, PACKET_MAGIC, PACKET_MAGIC_LEN) != 0) {
        DEBUG("invalid magic");
        return NULL;
    }

    // Expand escaped sequences.
    size_t escaped_payload_len = packet_len - PACKET_MAGIC_LEN;
    uint8_t *payload = start + 1 + PACKET_MAGIC_LEN;
    size_t w_i, r_i;
    for (w_i = 0, r_i = 0; r_i < escaped_payload_len; w_i++) {
        if (payload[r_i] == 0xee && r_i + 1 < escaped_payload_len) {
            // An escaped byte.
            switch (payload[r_i + 1]) {
            case 0xe0:
                payload[w_i] = 0x0a;
                break;
            case 0xe1:
                payload[w_i] = 0xee;
                break;
            default:
                DEBUG("invalid escape sequence");
                return NULL;
            }
            r_i += 2;
        } else {
            payload[w_i] = payload[r_i];
            r_i++;
        }
    }

    *payload_len = w_i;
    return payload;
}

static void send_packet(uint8_t *pkt, size_t pkt_len) {
   uart_write_bytes(UART_PORT, (const char *) pkt, pkt_len);
}

static void init_serial() {
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 122,
        .use_ref_tick = true,
    };

    uart_param_config(UART_PORT, &uart_config);
    uart_set_pin(UART_PORT, UART_TX_PIN, UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_PORT, BUF_LEN * 2, 0, 0, NULL, 0);
    uart_flush(UART_PORT);
}

void serial_adapter_task() {
    INFO("[Makestack] serial_adapter: starting");
    init_serial();

    uint8_t *buf = (uint8_t *) malloc(BUF_LEN);
    while (1) {
        uint8_t *payload;
        size_t payload_len;
        if ((payload = read_packet(buf, BUF_LEN, &payload_len)) == NULL) {
            adapter_read_error();
            continue;
        }

        process_payload(payload, payload_len);
        size_t pkt_len;
        if ((pkt_len = build_packet(buf, BUF_LEN)) == 0) {
            continue;
        }

        send_packet(buf, pkt_len);
    }
}
