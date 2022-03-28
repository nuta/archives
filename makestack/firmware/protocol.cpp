#include <makestack/types.h>
#include <makestack/logger.h>
#include <makestack/cred.h>
#include <makestack/logger.h>
#include <makestack/protocol.h>

#define MINIZ_NO_STDIO
#define MINIZ_NO_ARCHIVE_APIS
#define MINIZ_NO_TIME
#define MINIZ_NO_ARCHIVE_WRITING_APIS
#define MINIZ_NO_ZLIB_APIS
#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES
#define MINIZ_NO_MALLOC
#include "miniz.h"


enum class State {
    NORMAL,
    UPDATING,
};

static State current_state = State::NORMAL;
static uint32_t downloaded_size;
static uint64_t next_version = 0;
static esp_ota_handle_t update_handle;
static esp_partition_t *update_part;
static int read_retries = 0;
static bool reply_pong = false;

static uint16_t compute_checksum(uint8_t *buf, size_t len) {
    uint32_t checksum = 0;
    for (int i = 0; i < len; i++) {
        checksum += buf[i];
        checksum = checksum & 0xffff;
    }
    return checksum;
}


static int encode_leb128(uint8_t *buf, size_t buf_len, uint32_t value) {
    int i = 0;
    do {
        if (i >= buf_len) {
            WARN("too short buf");
            return 0;
        }

        uint8_t msb = (value >= 0x80) ? 0x80 : 0;
        buf[i] = msb | (value & 0x7f);
        i++;
        value >>= 7;
    } while (value > 0);

    return i;
}

static int decode_leb128(uint8_t *buf, size_t buf_len, uint64_t *value) {
    *value = 0;
    int shift = 0;
    for (int i = 0; i < buf_len && i < 4; i++) {
        *value |= (buf[i] & 0x7f) << shift;
        if ((buf[i] & 0x80) == 0) {
            return i + 1;
        }
        shift += 7;
    }

    // Invalid format.
    return -1;
}

static struct payload_header *verify_magic(uint8_t *payload, size_t payload_len) {
    struct payload_header *header = (struct payload_header *) payload;
    if (payload_len < sizeof(struct payload_header)) {
        DEBUG("payload is too short");
        return NULL;
    }

    if (header->len + sizeof(struct payload_header) > payload_len) {
        DEBUG("invalid payload len");
        return NULL;
    }

    uint8_t *data = payload + sizeof(struct payload_header);
    size_t data_len = payload_len - sizeof(struct payload_header);
    uint16_t checksum = compute_checksum(data, data_len);
    if (checksum != header->checksum) {
        WARN("invalid payload checksum (computed=%04x, expected=%04x)",
            checksum, header->checksum);
        return NULL;
    }

    return header;
}

static void update_firmware() {
    INFO("[MakeStack] Initiating software update...");
    current_state = State::UPDATING;
    downloaded_size = 0;

    update_part = const_cast<esp_partition_t *>(esp_ota_get_next_update_partition(NULL));
    if (update_part == NULL) {
        WARN("%s: failed to get next partition", __func__);
        return;
    }

    esp_err_t err = esp_ota_begin(update_part, OTA_SIZE_UNKNOWN, &update_handle);
    if (err != ESP_OK) {
        WARN("esp_ota_begin: %s", esp_err_to_name(err));
        return;
    }
}

struct version_info {
    uint64_t version;
} __attribute__((packed));

void process_version_info(uint8_t *data, size_t len) {
    if (len < sizeof(struct version_info)) {
        DEBUG("[MakeStack] too short data len");
        return;
    }

    struct version_info *info = (struct version_info *) data;
    DEBUG("[MakeStack] version_info=%llu", info->version);

    if (__cred.version < info->version && next_version != info->version) {
        next_version = info->version;
        update_firmware();
    }
}

static void abort_update() {
    current_state = State::NORMAL;
    downloaded_size = 0;
}

static void receive_ota_data(uint8_t *data, size_t len) {
    downloaded_size += len;
    esp_err_t err;
    if ((err = esp_ota_write(update_handle, data, len)) != ESP_OK) {
        WARN("esp_ota_write: %s", esp_err_to_name(err));
        abort_update();
        return;
    }
}

static void process_firmware_data(uint8_t *data, size_t len) {
    if (len < sizeof(struct firmware_data_header)) {
        DEBUG("[MakeStack] too short data len");
        return;
    }

    struct firmware_data_header *header = (struct firmware_data_header *) data;
    switch (header->type) {
    case FIRMWARE_DATA_EOF: {
        DEBUG("reached to the end of file, yay!");
        current_state = State::NORMAL;

        esp_err_t err;
        if ((err = esp_ota_end(update_handle)) != ESP_OK) {
            WARN("esp_ota_end: %s\n", esp_err_to_name(err));
            return;
        }

        if ((err = esp_ota_set_boot_partition(update_part)) != ESP_OK) {
            WARN("esp_ota_set_boot_partition: %s\n", esp_err_to_name(err));
            return;
        }

        INFO("restarting the system...");
        esp_restart();

        WARN("failed to restart :(");
        abort_update();
        break;
    }
    case FIRMWARE_DATA_RAW: {
        uint8_t *buf = data + sizeof(struct firmware_data_header);
        size_t buf_len = len - sizeof(struct firmware_data_header);
        receive_ota_data(buf, buf_len);
        break;
    }
    case FIRMWARE_DATA_DEFLATE: {
        uint8_t *buf = data + sizeof(struct firmware_data_header);
        size_t buf_len = len - sizeof(struct firmware_data_header);

        tinfl_decompressor decompressor;
        tinfl_init(&decompressor);
        size_t out_len = 32768;
        uint8_t *out = (uint8_t *) malloc(out_len);
        if (!out) {
            WARN("failed to free");
            return;
        }

        int flags = TINFL_FLAG_PARSE_ZLIB_HEADER;
        int status = tinfl_decompress(&decompressor, buf, &buf_len, out, out, &out_len, flags);
        if (status != TINFL_STATUS_DONE) {
            WARN("failed to decompress: status=%d", status);
            free(out);
            return;
        }

        receive_ota_data(out, out_len);
        free(out);
        break;
    }
    default:
        WARN("unknown firmware data type: %02x", header->type);
        abort_update();
        return;
    }
}

static void process_corrupt_rate_check(uint8_t *data, size_t data_len) {
    int num_corrupted = 0;
    uint8_t patterns[] = { 0x00, 0x5a, 0x0a, 0xff, 0xff, 0xa5, 0xee, 0xc0 };
    for (size_t i = 0; i < data_len; i++) {
        if (data[i] != patterns[i % 8]) {
            num_corrupted++;
        }
    }

    int perc = (num_corrupted * 100) / data_len;
    DEBUG("corrupt rate: %d%% (%d/%d bytes)", perc, num_corrupted, data_len);
}


static void process_ping(uint8_t *data, size_t data_len) {
    DEBUG("received ping (len=%d)", data_len);
    reply_pong = true;
}

static void process_field(uint8_t type, uint8_t *data, size_t data_len) {
    // DEBUG("type=%02x, len=%u", type, data_len);
    switch (type) {
    case 0x01: // latest version info
        process_version_info(data, data_len);
        break;
    case 0x02: // firmware data
        process_firmware_data(data, data_len);
        break;
    case 0x03: // corrupt rate check
        process_corrupt_rate_check(data, data_len);
        break;
    case 0x04: // ping
        process_ping(data, data_len);
        break;
    default:
        DEBUG("[MakeStack] Ignoring unknown type: %02x", type);
    }
}

void process_payload(uint8_t *payload, size_t payload_len) {
    struct payload_header *header;
    if ((header = verify_magic(payload, payload_len)) == NULL) {
        WARN("invalid payload");
        return;
    }

    uint8_t *p = payload + sizeof(struct payload_header);
    size_t remaining_len = payload_len - sizeof(struct payload_header);
    while (remaining_len > 2) {
        uint8_t type = p[0];
        uint64_t data_len;
        int len_len = decode_leb128(&p[1], remaining_len - 1, &data_len);
        if (len_len < 0) {
            DEBUG("invalid leb128");
            return;
        }

        uint8_t *data = p + 1 + len_len;
        process_field(type, data, data_len);

        size_t field_len = 1 + len_len + data_len;
        if (remaining_len < field_len)
            break;

        remaining_len -= field_len;
        p += field_len;
    }

    // We've successuflly received a packet. Reset the counter.
    read_retries = 0;
}

static size_t build_field(uint8_t *buf, size_t buf_len, uint8_t type, void *data, size_t data_len) {
    uint8_t leb128_len[4];
    size_t leb128_len_len = encode_leb128((uint8_t *) &leb128_len, sizeof(leb128_len), data_len);

    size_t header_len = 1 + leb128_len_len;
    if (data_len + header_len > buf_len) {
        return 0;
    }

    *buf++ = type;
    memcpy(buf, &leb128_len, leb128_len_len);
    buf += leb128_len_len;
    memcpy(buf, data, data_len);
    return header_len + data_len;
}

size_t build_device_status_field(uint8_t *&p, size_t& remaining) {
    struct device_status data;
    data.state = 0; // TODO:
    data.battery_level = 0; // TODO:
    data.ram_free = esp_get_free_heap_size();

    size_t copied_len;
    if (!(copied_len = build_field(p, remaining, 0x07, (void *) &data, sizeof(data)))) {
        return 0;
    }

    p += copied_len;
    remaining -= copied_len;
    return copied_len;
}

size_t build_log_field(uint8_t *&p, size_t& remaining) {
    size_t log_len;
    char *log = read_logger_buffer(&log_len);

    size_t copied_len;
    if (!(copied_len = build_field(p, remaining, 0x06, (void *) log, log_len))) {
        return 0;
    }

    p += copied_len;
    remaining -= copied_len;
    return copied_len;
}

size_t build_payload(uint8_t *buf, size_t buf_len) {
    size_t remaining = buf_len;
    uint8_t *p = buf;

    if (remaining < sizeof(struct payload_header)) {
        WARN("too short payload buf");
        return 0;
    }

    // We fill this later.
    struct payload_header *payload_header = (struct payload_header *) p;
    p += sizeof(struct payload_header);
    uint8_t *payload_data = p;

    if (!build_device_status_field(p, remaining)) {
        WARN("too short payload buf");
        return 0;
    }

    if (!build_log_field(p, remaining)) {
        WARN("too short payload buf");
        return 0;
    }

    // firmware_request
    if (current_state == State::UPDATING) {
        struct firmware_request data;
        data.offset = downloaded_size;
        data.version = next_version;

        size_t copied_len;
        if (!(copied_len = build_field(p, remaining, 0xaa, &data, sizeof(data)))) {
            WARN("too short payload buf");
            return 0;
        }

        p += copied_len;
        remaining -= copied_len;
    }

    // pong
    if (reply_pong) {
        size_t copied_len;
        if (!(copied_len = build_field(p, remaining, 0x05, (void *) PONG_DATA, PONG_DATA_LEN))) {
            WARN("too short payload buf");
            return 0;
        }

        p += copied_len;
        remaining -= copied_len;
        reply_pong = false;
    }

    size_t payload_len = (uintptr_t) p - (uintptr_t) buf;
    size_t payload_data_len = payload_len - sizeof(struct payload_header);
    payload_header->checksum = compute_checksum(payload_data, payload_data_len);
    payload_header->len = payload_data_len;
    return payload_len;
}

void adapter_read_error() {
    // FIXME: handle this in a smart way.
    if (current_state == State::UPDATING) {
        if (read_retries > 5) {
            WARN("too many retries, aborting the firmware update");
            abort_update();
        } else {
            TRACE("failed read a packet, retyring....");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            read_retries++;
        }
    }
}
