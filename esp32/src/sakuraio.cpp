#include <esp_ota_ops.h>
#include "makestack.h"
#include "sakuraio.h"
#include "telemata.h"
#include "logger.h"
#include "utils.h"
#include "BinaryStream.h"

// sakura.io allows to send 16 channels at a time. 16th one is used
// for CHANNEL_COMMIT.
#define SAKURAIO_I2C_ADDR 0x4f
#define CHANNELS_MAX 15
#define CHANNEL_COMMIT 16
#define BYTES_PER_CHANNEL 8
#define CH_TYPE_8BYTES 0x62
#define PACKET_LEN_MAX (CHANNELS_MAX + BYTES_PER_CHANNEL)
#define CONNECTION_READY 0x80
#define CMD_RESULT_SUCCESS 0x01
#define CMD_RESULT_PROCESSING 0x07
#define CMD_GET_CONNECTION_STATUS 0x01
#define CMD_START_FILE_DOWNLOAD 0x40
#define CMD_GET_FILE_METADATA 0x41
#define CMD_GET_DOWNLOAD_STATUS 0x42
#define CMD_GET_FILE_DATA 0x44
#define CMD_TX_ENQUEUE 0x20
#define CMD_TX_SEND 0x24
#define CMD_RX_DEQUEUE 0x30
#define CMD_RX_LENGTH 0x32
#define CMD_FLUSH_RX_QUEUE 0x33
#define FILE_CHUNK_SIZE 64
#define GET_FILE_DATA_RETIES_MAX 30
#define APP_IMAGE_FILEID 1

SakuraioTelemataClient::SakuraioTelemataClient(const char *device_name)
    : TelemataClient(device_name), connected(false), downloading(false) {
}

void SakuraioTelemataClient::connect() {
    while (true) {
        INFO("sakura.io: connecting to sakura.io");
        if (get_connection_status() & CONNECTION_READY) {
            break;
        }

        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }

    connected = true;
    INFO("connected to sakura.io");
}

uint8_t SakuraioTelemataClient::get_connection_status() {
    uint8_t resp[1];
    send_command(CMD_GET_CONNECTION_STATUS, nullptr, 0, resp, sizeof(resp));
    return resp[0];
}

void SakuraioTelemataClient::enqueue_tx(int ch, int type, void *buf, int len) {
    if (len > 8) {
        INFO("too long queue buf");
        return;
    }

    uint8_t request[10]; /* ch, type, buf (time offset is not included) */
    request[0] = ch;
    request[1] = type;
    memcpy((void *) &request[2], buf, len);

    INFO("enqueue: ch=%d, type=%d, r=%x %x %x", ch, type, request[2], request[3], request[4]);
    send_command(CMD_TX_ENQUEUE, (uint8_t *) &request, sizeof(request), nullptr, 0);
}

int SakuraioTelemataClient::dequeue_rx(uint8_t *buf, int len) {
    uint8_t resp[18]; /* ch, type, buf, and time offset */
    int ret = send_command(CMD_RX_DEQUEUE, nullptr, 0, (uint8_t *) &resp, sizeof(resp));
    if (ret < 0) {
        return ret;
    }

    int ch = resp[0];
    memcpy((void *) buf, &resp[2], len);
    return ch;
}

int SakuraioTelemataClient::request_file_download(int id) {
    uint8_t req[2];
    req[0] = id & 0xff;
    req[1] = (id >> 8) & 0xff;
    return send_command(CMD_START_FILE_DOWNLOAD, (uint8_t *) req, sizeof(req), nullptr, 0);
}

int SakuraioTelemataClient::get_file_size() {
    uint8_t resp[17];
    send_command(CMD_GET_FILE_METADATA, nullptr, 0, (uint8_t *) resp, sizeof(resp));
    return (resp[1] & 0xff) | ((resp[2] & 0xff) << 8) | ((resp[3] & 0xff) << 16) | ((resp[4] & 0xff) << 24);
}

bool SakuraioTelemataClient::is_downloading_file() {
     uint8_t resp[17];
    int r = send_command(CMD_GET_FILE_METADATA, nullptr, 0, (uint8_t *) resp, sizeof(resp));
    INFO("r=%d, %x", r, resp[0]);

    return r != sizeof(resp) || resp[0] == 0x83;
}

void SakuraioTelemataClient::print_download_status() {
    uint8_t resp[17];
    send_command(CMD_GET_DOWNLOAD_STATUS, nullptr, 0, (uint8_t *) resp, sizeof(resp));
    INFO("sakura.io: downloaded %d bytes", (resp[1] & 0xff) | ((resp[2] & 0xff) << 8) | ((resp[3] & 0xff) << 16) | ((resp[4] & 0xff) << 24));
}

int SakuraioTelemataClient::get_file_data(uint8_t *buf, int len) {
    uint8_t req_len = len;
    return send_command(CMD_GET_FILE_DATA, &req_len, 1, buf, len);
}

int8_t SakuraioTelemataClient::get_rx_queue_length() {
    uint8_t resp[2];
    send_command(CMD_RX_LENGTH, nullptr, 0, resp, sizeof(resp));
    return resp[1];
}

uint8_t SakuraioTelemataClient::flush_rx_queue() {
    uint8_t resp[1];
    send_command(CMD_FLUSH_RX_QUEUE, nullptr, 0, nullptr, 0);
    return resp[0];
}

void SakuraioTelemataClient::flush_tx_queue() {
    send_command(CMD_TX_SEND, nullptr, 0, nullptr, 0);
}

uint8_t compute_parity(int first_byte, const uint8_t *data, int len) {
    uint8_t parity = first_byte ^ len;
    for (int i = 0; i < len; i++) {
        parity ^= data[i];
    }

    return parity;
}

/* Return the received length on success or a negative value on failure. */
int SakuraioTelemataClient::send_command(int cmd, uint8_t *data, int len, uint8_t *resp, uint8_t resp_len) {
    DEBUG("execute: cmd=%x, len=%d:%d", cmd, len, resp_len);

    // Construct a request.
    int request_len = 2 + len + 1; /* cmd, len, data, and parity */
    uint8_t *request = (uint8_t *) malloc(request_len);
    request[0] = cmd;
    request[1] = len;
    memcpy((void *) &request[2], data, len);
    request[2 + len] = compute_parity(cmd, data, len);

    // Send the request.
    Wire.beginTransmission(SAKURAIO_I2C_ADDR);
    for (int i = 0; i < request_len; i++) {
        Wire.write(request[i]);
    }

    // The specification requires this.
    Wire.endTransmission();
    vTaskDelay(20 / portTICK_PERIOD_MS);

    // Receive a response from the module.
    Wire.requestFrom(
        (uint8_t) SAKURAIO_I2C_ADDR,
        (size_t) 3 /* result, len, and parity */ + resp_len,
        true
    );

    int result = Wire.read();
    if (result != CMD_RESULT_SUCCESS) {
        ERROR("sakura.io: module returned %d", result);
        free(request);
        return - ((int) result);
    }

    int read_len = Wire.read();
    if (read_len > resp_len) {
        ERROR("sakura.io: resp buffer is too short (cmd=%x, resp_len=%d)", cmd, resp_len);
        free(request);
        return -1;
    }

    for (int i = 0; i < read_len; i++) {
        resp[i] = Wire.read();
    }

    int resp_parity = Wire.read();
    if (resp_parity != compute_parity(result, resp, read_len)) {
        ERROR("sakura.io: parity mismatch");
        free(request);
        return -1;
    }

    DEBUG("cmd success: cmd=%x, r=%d, l=%d", cmd, result, read_len);
    free(request);
    return read_len;
}

void SakuraioTelemataClient::send_payload(const void *payload, size_t length) {
    if (!connected || downloading) {
        return;
    }

    if (length > CHANNELS_MAX * BYTES_PER_CHANNEL) {
        WARN("sakura.io: too long payload, ignoring...");
        return;
    }

    INFO("sending over sakura.io");
    hexdump((uint8_t *) payload, length);
    // Split the payload into 8 bytes (BYTES_PER_CHANNEL) arrays.
    for (int i = 0; 0 < length && i < CHANNELS_MAX; i++) {
        uint8_t buf[BYTES_PER_CHANNEL];
        int copy_len = (length < BYTES_PER_CHANNEL) ? length : BYTES_PER_CHANNEL;
        memset(&buf, 0, sizeof(buf));
        memcpy(&buf, (void *) ((uintptr_t) payload + (i * BYTES_PER_CHANNEL)), copy_len);
        enqueue_tx(i, CH_TYPE_8BYTES, &buf, sizeof(buf));
        length -= copy_len;
    }

    flush_tx_queue();
}

void SakuraioTelemataClient::receive_forever() {
    flush_rx_queue();
    uint8_t *payload = (uint8_t *) malloc(CHANNELS_MAX * BYTES_PER_CHANNEL);
    while (true) {
        if (!connected || downloading) {
            vTaskDelay(5000 / portTICK_PERIOD_MS);
            continue;
        }

        for (int i = 0; i < CHANNELS_MAX + 1 /* including CHANNEL_COMMIT */; i++) {
retry:
            int queued_len = get_rx_queue_length();
            if (!connected || queued_len <= 0) {
                DEBUG("received queue_len=%d", queued_len);
                vTaskDelay(3000 / portTICK_PERIOD_MS);
                continue;
            }

            uint8_t buf[BYTES_PER_CHANNEL];
            int channel = dequeue_rx((uint8_t *) &buf, sizeof(buf));

            if (channel < 0) {
                WARN("sakura.io: failed to dequeue (%d)", channel);
                vTaskDelay(500 / portTICK_PERIOD_MS);
                goto retry;
            }

            if (channel == CHANNEL_COMMIT) {
                int payload_len = i * BYTES_PER_CHANNEL;
                INFO("sakuraio: received payload len=%d", payload_len);
                hexdump(payload, payload_len);
                receive_payload(payload, payload_len);
                break;
            }

            if (channel == 0) {
                // Assuming that this value is the beginning of a payload.
                i = 0;
            }

            if (channel != i) {
                INFO("sakura.io: skipping old data ch=%d", channel);
                goto retry;
            }

            INFO("received a ch: %d", channel);
            memcpy((void *) &payload[i * BYTES_PER_CHANNEL], &buf, sizeof(buf));
        }
    }
}

struct firmware_header {
    uint8_t magic[4];
    uint32_t length;
    uint32_t version;
    uint32_t reserved;
} __attribute__((packed));

void SakuraioTelemataClient::download_app(int version) {
    ERROR("sakura.io: download_app is not yet implemented");
    /* Before completing the implementation we needs delta update to minimize the
       size of update data; this takes long time. */
#if 0
    INFO("sakura.io: initiating a file download...");
    downloading = true;
    request_file_download(APP_IMAGE_FILEID);
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    // Wait until the module finish downloading the app image file.
    while (true) {
        INFO("sakura.io: downloading...");
        print_download_status();

        if (!is_downloading_file()) {
            break;
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    INFO("sakura.io: successfully downloaded the firmware");
    int filesize = get_file_size();
    if (filesize == 0) {
        ERROR("sakura.io: failed to download a file");
        downloading = false;
        return;
    }

    INFO("sakura.io: file size is %d bytes", filesize);

    esp_ota_handle_t update_handle;
    const esp_partition_t *part;
    part = esp_ota_get_next_update_partition(NULL);
    if (part == NULL) {
        printf("%s: failed to get next partition\n", __func__);
        downloading = false;
        return;
    }

    esp_err_t err = esp_ota_begin(part, OTA_SIZE_UNKNOWN, &update_handle);
    if (err != ESP_OK) {
        printf("esp_ota_begin: %s\n", esp_err_to_name(err));
        downloading = false;
        return;
    }

    struct firmware_header header;
    int len;
    int retries;
    for (retries = 0; retries < GET_FILE_DATA_RETIES_MAX; retries++) {
        if ((len = get_file_data((uint8_t *) &header, sizeof(header))) > 0) {
            break;
        }

        WARN("sakura.io: get_file_data() returned an error, retrying....");
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }

    if (retries == GET_FILE_DATA_RETIES_MAX) {
        INFO("failed to read the file");
        downloading = false;
        return;
    }

    if (len < sizeof(header)) {
        INFO("too short firmware");
        downloading = false;
        return;
    }

    DEBUG("sakura.io: read the firmware header");
    hexdump((uint8_t *) &header, sizeof(header));

    if (header.magic[0] != 0x81 || header.magic[1] != 0xf1
        || header.magic[2] != 0x5c || header.magic[3] != 0x5e) {
        INFO("invalid firmware signature");
        downloading = false;
        return;
    }

    int offset = sizeof(header);
    uint8_t *buf = (uint8_t *) malloc(FILE_CHUNK_SIZE);
    while (offset < filesize) {
        if ((len = get_file_data(buf, FILE_CHUNK_SIZE)) <= 0) {
            WARN("sakura.io: get_file_data() returned an error, retrying in a sec....");
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        printf("Downloading %d/%d\n", offset, filesize);
        if (len < FILE_CHUNK_SIZE) {
            if (offset + len == filesize) {
                len = FILE_CHUNK_SIZE;
            } else {
                WARN("len is too short (%d)", len);
            }
        }

        if ((err = esp_ota_write(update_handle, buf, len)) != ESP_OK) {
            printf("esp_ota_write: %s\n", esp_err_to_name(err));
            free(buf);
            downloading = false;
            return;
        }

        offset += len;
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }

    free(buf);

    if (offset < header.length) {
        printf("disconnected\n");
        downloading = false;
        return;
    }

    if (esp_ota_end(update_handle) != ESP_OK) {
        printf("esp_ota_end: %s\n", esp_err_to_name(err));
        downloading = false;
        return;
    }

    if ((err = esp_ota_set_boot_partition(part)) != ESP_OK) {
        printf("esp_ota_set_boot_partition: %s\n", esp_err_to_name(err));
        downloading = false;
        return;
    }

    printf("restarting the system...\n");
    esp_restart();
    printf("failed to restart :(\n");
#endif
}
