/*
* A sakura.io communication module adapter.
*
* Refer: https://sakura.io/docs (Google Translate is your friend)
*
*/
import { I2C, Timer } from "makestack";
import * as logger from "../logger";
import { AdapterBase } from "./adapter_base";

// 1 - 19999 are reserved for OS images.
const APP_IMAGE_FILEID = 1;
// sakura.io allows to send 16 channels at a time. 16th one is used
// for CHANNEL_COMMIT.
const CHANNELS_MAX = 15;
const CHANNEL_COMMIT = 16;
const BYTES_PER_CHANNEL = 8;
const CH_TYPE_8BYTES = 0x62;
const PACKET_LEN_MAX = CHANNELS_MAX * BYTES_PER_CHANNEL;
const CONNECTION_READY = 0x80;
const CMD_RESULT_SUCCESS = 0x01;
const CMD_RESULT_PROCESSING = 0x07;
const CMD_GET_CONNECTION_STATUS = 0x01;
const CMD_START_FILE_DOWNLOAD = 0x40;
const CMD_GET_FILE_METADATA = 0x41;
const CMD_GET_FILE_DATA = 0x44;
const CMD_TX_ENQUEUE = 0x20;
const CMD_TX_SEND = 0x24;
const CMD_RX_DEQUEUE = 0x30;
const CMD_RX_LENGTH = 0x32;
const FILE_CHUNK_SIZE = 16;

export type TChannel = number;
export type TChannelType = 0x62 /* CH_TYPE_8BYTES */;

export abstract class SakuraIODriverBase {
    public abstract command(command: number, data: Buffer): Promise<[number, Buffer]>;

    public computeParity(firstByte: number, data: Buffer) {
        let parity = firstByte ^ data.length;
        for (const byte of data) {
            parity ^= byte;
        }

        return parity;
    }

    public async enqueueTx(channel: TChannel, type: TChannelType, value: Buffer) {
        if (value.length > BYTES_PER_CHANNEL) {
            throw new Error("BUG: sakura.io: enqueue value must be equal to or less than 8 bytes");
        }

        const request = Buffer.alloc(BYTES_PER_CHANNEL + 2);
        request.writeUInt8(channel, 0);
        request.writeUInt8(type, 1);
        value.copy(request, 2);

        await this.command(CMD_TX_ENQUEUE, request);
    }

    public async flushTx() {
        await this.command(CMD_TX_SEND, Buffer.alloc(0));
    }

    public async dequeueRx(): Promise<[number, Buffer]> {
        const [, resp] = await this.command(CMD_RX_DEQUEUE, Buffer.alloc(0));
        const channel = resp.readUInt8(0);

        const data = Buffer.alloc(BYTES_PER_CHANNEL);
        for (let i = 0; i < BYTES_PER_CHANNEL; i++) {
            data[i] = resp.readUInt8(2 + i);
        }

        return [channel, data];
    }

    public async getRxQueueLength() {
        const [, resp] = await this.command(CMD_RX_LENGTH, Buffer.alloc(0));
        const available = resp.readUInt8(0);
        const queued = resp.readUInt8(1);

        return [available, queued];
    }

    public async requestFileDownload(fileId: number) {
        const buf = Buffer.alloc(2);
        buf.writeUInt16LE(fileId, 0);
        await this.command(CMD_START_FILE_DOWNLOAD, buf);
    }

    public async isDownloadingFile() {
        const [result] = await this.command(CMD_GET_FILE_METADATA, Buffer.alloc(0));
        return (result !== CMD_RESULT_SUCCESS);
    }

    public async getFileMetadata() {
        const [, response] = await this.command(CMD_GET_FILE_METADATA, Buffer.alloc(0));
        const status = response.readUInt8(0);
        const size = response.readUInt32LE(1);
        const timestamp = response.readUInt32LE(5);
        const crc = response.readUInt8(13);
        return [status, size, timestamp, crc];
    }

    public async getFileData(chunkSize: number) {
        return this.command(CMD_GET_FILE_DATA, Buffer.from([chunkSize]));
    }

    public async getFileDownloadStatus() {
        return this.command(0x42, Buffer.alloc(0));
    }

    public async getConnectionStatus() {
        const [result, resp] = await this.command(CMD_GET_CONNECTION_STATUS, Buffer.alloc(0));
        return (result === CMD_RESULT_SUCCESS) ? resp.readUInt8(0) : 0x7f;
    }
}

export class I2CSakuraIODriver extends SakuraIODriverBase {
    public i2c: any;

    constructor() {
        super();
        this.i2c = new I2C({ address: 0x4f });
    }

    public async command(command: number, data: Buffer): Promise<[number, Buffer]> {
        // Send a request.
        const request = Buffer.alloc(2 + data.length + 1);
        request.writeUInt8(command, 0);
        request.writeUInt8(data.length, 1);
        data.copy(request, 2);
        request.writeUInt8(this.computeParity(command, data), 2 + data.length);

        this.i2c.write(request);

        Timer.busywait(10 * 1000);

        // Receive a response from the module.
        const buf            = (await this.i2c.read(32));
        const result         = buf.readUInt8(0);
        const responseLength = buf.readUInt8(1);
        const parity         = buf.readUInt8(responseLength + 2);
        const response       = buf.slice(2, responseLength + 2);

        if (result !== CMD_RESULT_SUCCESS) {
            logger.warn(`sakura.io: module returned ${result}`);
            return [result, Buffer.alloc(0)];
        }

        if (parity !== this.computeParity(result, response)) {
            logger.error("sakura.io: parity mismatch");
            return [result, Buffer.alloc(0)]; // FIXME
        }

        return [result, response];
    }
}

export class SakuraIOAdapter extends AdapterBase {
    public sakuraio: I2CSakuraIODriver;
    public received: Buffer[];

    constructor() {
        super();
        this.sakuraio = new I2CSakuraIODriver();
        this.received = [];
    }

    public async connect() {
        // Wait until the module gets connected.
        while (true) {
            logger.debug("sakuraio: connecting...");
            if ((await this.sakuraio.getConnectionStatus()) & CONNECTION_READY) {
                break;
            }

            await Timer.sleep(1);
        }

        logger.debug("sakuraio: connected!");
    }

    public async receive() {
        // Receive payloads from sakura.io.
        const [, queued] = await this.sakuraio.getRxQueueLength();
        if (queued > 0) {
            let commited = false;
            for (let i = 0; i < queued; i++) {
                const [channel, data] = await this.sakuraio.dequeueRx();
                if (channel === CHANNEL_COMMIT) {
                    commited = true;
                    break;
                }

                this.received[channel] = data;
            }

            // Received whole payload.
            if (commited) {
                const payload = Buffer.alloc(PACKET_LEN_MAX);
                for (let i = 0; i < CHANNELS_MAX; i++) {
                    if (this.received[i]) {
                        if (i < 0 && this.received[i - 1] === undefined && this.received[i]) {
                            logger.error("sakuraio: detected a loss of channel");
                            return;
                        }

                        this.received[i].copy(payload, i * BYTES_PER_CHANNEL);
                    }
                }

                this.received = [];
                logger.debug("sakuraio: received payload", payload);
                this.onReceiveCallback(payload);
            }
        }
    }

    public doSend(payload: Buffer) {
        // Split the payload into 8 bytes (BYTES_PER_CHANNEL) arrays.
        const channels = [];
        for (let i = 0; i < payload.length; i += BYTES_PER_CHANNEL) {
            channels.push(payload.slice(i, i + BYTES_PER_CHANNEL));
        }

        if (channels.length > CHANNELS_MAX) {
            throw new Error("sakura.io: # of channel must be less than 16");
        }

        for (const [i, ch] of channels.entries()) {
            this.sakuraio.enqueueTx(i, CH_TYPE_8BYTES, ch);
        }

        this.sakuraio.flushTx();
    }

    public async send(payload: Buffer) {
        this.doSend(payload);
        this.receive();
    }

    public async getAppImage(version: string) {
        logger.info("sakura.io: requesting a file download....");
        await this.sakuraio.requestFileDownload(APP_IMAGE_FILEID);

        // Wait until the module finish downloading the app image file.
        while (true) {
            if (!(await this.sakuraio.isDownloadingFile())) {
                break;
            }

            await Timer.sleep(1);
        }

        const [, fileSize] = await this.sakuraio.getFileMetadata();
        if (fileSize === 0) {
            throw new Error("sakura.io: failed to download a file");
        }

        const appImage = Buffer.alloc(fileSize);
        let offset = 0;
        while (offset < fileSize) {
            const [result, data] = await this.sakuraio.getFileData(FILE_CHUNK_SIZE);
            if (result !== CMD_RESULT_SUCCESS) {
                logger.debug("sakura.io: getFileData returned an error, retrying in a sec....");
                await Timer.sleep(1);
                continue;
            }

            data.copy(appImage, offset);
            offset += data.length;
            const perc = ((offset / fileSize) * 100).toFixed(2);
            logger.debug(`sakura.io: received ${offset} bytes ${perc}%`);
        }

        logger.debug("sakura.io: Hooray! You got a new app image!");
        return Promise.resolve(appImage);
    }

    public getOSImage(version: string) {
        return Promise.reject("sakura.io: getOSImage is not supported yet");
    }
}
