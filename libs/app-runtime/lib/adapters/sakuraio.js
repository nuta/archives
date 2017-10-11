/*
 * A sakura.io communication module adapter.
 *
 * Refer: https://sakura.io/docs (Send them a feedback if you cannot understand Japanese)
 * Feedback Form: https://www.sakura.ad.jp/request_form/service/iot (Google Translate is your friend)
 *
 */
const Driver = require('./driver')

// 1 - 19999 are reserved for OS images.
const APP_IMAGE_FILEID_OFFSET = 20000
// sakura.io allows to send 16 channels at a time.
const CHANNELS_MAX = 16
const BYTES_PER_CHANNEL = 8
const PACKET_LEN_MAX = CHANNELS_MAX * BYTES_PER_CHANNEL
const CONNECTION_READY = 0x80
const CMD_ERROR_NONE = 0x01
const CMD_GET_CONNECTION_STATUS = 0x01
const CMD_START_FILE_DOWNLOAD = 0x40
const CMD_GET_FILE_METADATA = 0x41
const CMD_GET_FILE_DATA = 0x44
const CMD_TX_ENQUEUE = 0x20
const CMD_TX_SEND = 0x24
const CMD_RX_DEQUEUE = 0x30
const CMD_RX_LENGTH = 0x32

class SakuraIODriverBase extends Driver {
  computeParity(firstByte, data) {
    let parity = firstByte ^ request.length
    for (let i = 0; i < request.length; i++) {
      parity ^= request[i]
    }

    return parity
  }

  enqueueTx(channel, value) {
    let request = Buffer([channel, type] + value)
    if (request.length != 10)
      throw 'BUG: sakura.io: request must be 10 bytes';

    await this.command(CMD_TX_ENQUEUE, request)
  }

  flushTx() {
    await this.command(CMD_TX_SEND, Buffer.alloc(0))
  }

  dequeueRx() {
    let result, resp = await this.command(CMD_RX_DEQUEUE, Buffer.alloc(0))
    let channel = resp.readUInt8(0)

    let data = Buffer.alloc(BYTES_PER_CHANNEL)
    for (let i = 0; i < BYTES_PER_CHANNEL; i++) {
      data[i]  = resp.readUInt8(2 + i)      
    }

    return channel, data
  }

  getRxQueueLength() {
    let result, resp = await this.command(CMD_RX_LENGTH, Buffer.alloc(0))
    let available = resp.readUInt8(0)
    let queued    = resp.readUInt8(1)

    return available, queued
  }

  requestFileDownload(fileId) {
    let buf = Buffer.alloc(2)
    buf.writeUInt16LE(fileId, 0)
    await this.command(CMD_START_FILE_DOWNLOAD, buf)
  }

  requestFileDownload(fileId) {
    let buf = Buffer.alloc(2)
    buf.writeUInt16LE(fileId, 0)
    await this.command(CMD_START_FILE_DOWNLOAD, buf)
  }

  getFileMetadata() {
    let result, response = this.command(CMD_GET_FILE_METADATA, Buffer.alloc(0))
    let status    = response.readUInt8(0)
    let size      = response.readUInt32LE(1)
    let timestamp = response.readUInt64LE(5)
    let crc       = response.readUInt8(13)
    return status, size, timestamp, crc
  }

  getFileData() {   
    return this.command(CMD_GET_FILE_DATA, Buffer([16]))
  }

  getConnectionStatus() {
    let result, resp = await this.command(CMD_GET_CONNECTION_STATUS, Buffer.alloc(0))
    let status = resp.readUInt8(0)

    return (result == CMD_ERROR_NONE) ? status : 0x7f;
  }
}

class I2CSakuraIODriver extends SakuraIODriverBase {
  constructor(apis, i2cAddress) {
    this.apis = apis
    this.addr = i2cAddress
  }

  async command(command, data) {
    const i2c = this.apis.i2c;

    // Send a request.
    let request = [command, request.length] + data + [computeParity(command, request)]
    i2c.write(this.addr, new Buffer(request))

    await this.delay(10)

    // Receive a response from the module.
    let result = i2c.read(this.addr, 1)
    let responseLength = i2c.read(this.addr, 1)
    let parity = i2c.read(this.addr, 1)
    let response = i2c.read(this.addr, responseLength)

    if (result != CMD_ERROR_NONE) {
      console.warn(`sakura.io: module returned ${command}`)
      return result
    }

    if (parity != computeParity(result, response)) {
      this.apis.logging.error("sakura.io: parity mismatch");
      return
    }

    return result, response
  }
}

class SakuraIOAdapter {
  constructor(apis, i2cAddress) {
    // TODO: support SPI
    this.sakuraio = new I2CSakuraIODriver(apis)
    this.connect()
    this.startReceiving()
  }

  async connect() {
    // Wait until the module gets connected.
    while (true) {
      console.log('sakuraio: connecting...')
      if (this.sakuraio.getConnectionStatus() & CONNECTION_READY)
        break
      
      await this.delay(1000)
    }
    console.log('sakuraio: connected!')
  }

  startReceiving() {
    // Receive payloads from sakura.io.
    let received = []
    setInterval(() => {
      let available, queued = this.sakuraio.getRxQueueLength()
      if (queued > 0) {
        let commited = false
        for (let i = 0; i < queued; i++) {
          let channel, data = this.sakuraio.dequeueRx()
          console.log(`sakuraio: received channel=${channel} data=${data}`)
          if (channel == CHANNEL_COMMIT) {
            commited = true
            break  
          }

          received[channel] = data
        }

        // Received whole payload.
        if (commited) {
          let payload = Buffer.alloc(PACKET_LEN_MAX)
          for (let i = 0; i < CHANNELS_MAX; i++) {
            if (received[i] === undefined) {
              console.error('sakuraio: detected a loss of channel')
              return
            }
            
            received[i].copy(payload, i * BYTES_PER_CHANNEL)
          }

          received = []
          this.onReceiveCallback(this.deserialize(payload))
        }
      }
    }, 15 * 1000)
  }

  doSend(payload) {
    // Split the payload into 8 bytes (BYTES_PER_CHANNEL) arrays.
    let channels = []
    for (let i = 0; i < payload.length; i += BYTES_PER_CHANNEL) {
      channels.push(payload.slice(i, i + BYTES_PER_CHANNEL))
    }

    if (channels.length > CHANNELS_MAX)
      throw 'sakura.io: # of channel must be less than 16';

    for (let i = 0; i < channels.length; i++) {
      this.sakuraio.enqueueTx(i, channels[i])
    }

    this.sakuraio.flushTx()
  }

  send(messages) {
    let withoutLog = messages.filter(k => k != 'log')
    let withoutLogLength = this.serialize(withoutLog).length
    if (withoutLogLength > PACKET_LEN_MAX)
      throw 'too many messages'

    // Shorten log to satisfy PACKET_LEN_MAX.
    const MSGPACK_LOG_HEADER = 2 // Message Pack str8 header
    let logLength = PACKET_LEN_MAX - MSGPACK_LOG_HEADER - withoutLogLength
    let shortenedMessage = Object.assign({}, withoutLog)
    shrotenedMessage.log = messages['log'].substring(0, logLength)
    let payload = msgpack.pack(shortenedMessage)

    this.doSend(payload)
  }

  async getAppImage(version) {
    console('sakura.io: requesting a file download....')
    this.sakuraio.requestFileDownload(APP_IMAGE_FILEID_OFFSET + version)

    // We assume that file downloading is slow. delay for a while.
    await this.delay(500)
      
    // Wait until the module finish downloading the app image file.
    let status, fileSize, timestamp, crc
    while (true) {
      console('sakura.io: check file metadata')
      status, fileSize, timestamp, crc = this.sakuraio.getFileMetadata()
      if (status == CMD_ERROR_NONE)
        break

      await this.delay(1000)
    }

    let appImage = Buffer.alloc(fileSize)
    let offset = 0
    while (offset < fileSize) {
      let result, data = this.sakuraio.getFileData(FILE_CHUNK_SIZE)
      if (result != CMD_ERROR_NONE) {
        console.log('sakura.io: getFileData returned an error, retrying in 500 msec....')
        await this.delay(500)
        continue
      }

      appImage.from(data, offset)
      offset += data.length
      let perc = (offset / data.length) * 100
      console.log(`sakura.io: received ${offset} bytes %{perc}%`)
    }

    console.log(`sakura.io: Hooray! You got a new app image!`)
    return Promise.resolve(appImage)
  }
}