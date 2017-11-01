const SMMS_VERSION = 1
const SMMS_DEVICE_ID_MSG = 0x0a
const SMMS_DEVICE_INFO_MSG = 0x0b
const SMMS_LOG_MSG = 0x0c
const SMMS_OS_VERSION_MSG = 0x10
const SMMS_APP_VERSION_MSG = 0x11
const SMMS_STORE_MSG = 0x40

class AdapterBase {
  constructor() {
    this.onReceiveCallback = () => { }
  }

  onReceive(callback) {
    this.onReceiveCallback = callback
  }

  generateVariableLength(buf) {
    let len = buf.length
    let lenbuf = Buffer.alloc(0)

    while (len > 0) {
      const digit = len % 0x80
      len = Math.floor(len / 0x80)
      buf = Buffer.from([((len > 0) ? 0x80 : 0) | digit])
      lenbuf = Buffer.concat([lenbuf, buf])
    }

    return ((lenbuf.length > 0) ? lenbuf : Buffer.from([0x00]))
  }

  parseVariableLength(buf) {
    let length = 0
    let i = 0
    let base = 1
    while (true) {
      if (i === buf.length) {
        throw new Error('invalid variable length')
      }

      const byte = buf[i]
      length += (byte & 0x7f) * base

      if ((byte & 0x80) === 0) {
        return [length, i + 1]
      }

      i++
      base *= 128
    }
  }

  generateMessage(type, payload) {
    const buf = Buffer.from(payload)
    const lenbuf = this.generateVariableLength(buf)
    const msg = Buffer.alloc(1 + buf.length + 1)
    msg.writeUInt8(type, 0)
    lenbuf.copy(msg, 1) // size
    buf.copy(msg, 1 + lenbuf.length)

    return msg
  }

  serialize(messages) {
    let payload = Buffer.alloc(0)

    if ('deviceId' in messages) {
      const deviceIdMsg = this.generateMessage(SMMS_DEVICE_ID_MSG, messages.deviceId)
      payload = Buffer.concat([payload, deviceIdMsg])
    }

    if ('state' in messages) {
      const states = { booting: 1, ready: 2, running: 3 }

      if (!states[messages.state]) {
        throw new Error(`Invalid device state: \`${messages.state}'`)
      }

      const data = [states[messages.state]]
      const deviceInfoMsg = this.generateMessage(SMMS_DEVICE_INFO_MSG, data)

      payload = Buffer.concat([payload, deviceInfoMsg])
    }

    if ('log' in messages) {
      const logMsg = this.generateMessage(SMMS_LOG_MSG, messages.log)
      payload = Buffer.concat([payload, logMsg])
    }

    if ('osVersion' in messages) {
      const osVersionMsg = this.generateMessage(SMMS_OS_VERSION_MSG, messages.osVersion)
      payload = Buffer.concat([payload, osVersionMsg])
    }

    if ('appVersion' in messages) {
      const appVersionMsg = this.generateMessage(SMMS_APP_VERSION_MSG, messages.appVersion)
      payload = Buffer.concat([payload, appVersionMsg])
    }

    let header = Buffer.alloc(1)
    header.writeUInt8(SMMS_VERSION << 4, 0)
    header = Buffer.concat([header, this.generateVariableLength(payload)])

    return Buffer.concat([header, payload])
  }

  deserialize(payload) {
    let version = payload.readUInt8(0)
    if (version >> 4 !== SMMS_VERSION) {
      throw new Error('unsupported smms version')
    }

    const [totalLength, totalLengthLength] = this.parseVariableLength(payload.slice(1))
    const headerLength = 1 + totalLengthLength
    let messages = {}
    let offset = headerLength
    while (offset < headerLength + totalLength) {
      const type = payload[offset]
      const [length, lengthLength] = this.parseVariableLength(payload.slice(offset + 1))
      const dataOffset = offset + 1 + lengthLength
      const data = payload.slice(dataOffset, dataOffset + length)

      switch (type) {
        case SMMS_STORE_MSG: {
          const keyLengthOffset = dataOffset
          const [keyLength, keyLengthLength] = this.parseVariableLength(payload.slice(keyLengthOffset))
          const keyOffset = keyLengthOffset + keyLengthLength
          const valueOffset = keyOffset + keyLength
          const valueLength = length - (valueOffset - keyLengthOffset)
          const key = payload.slice(keyOffset, keyOffset + keyLength)
          const value = payload.slice(valueOffset, valueOffset + valueLength)

          if (!('stores' in messages)) {
            messages.stores = {}
          }

          messages.stores[key] = value
          break
        }
        case SMMS_OS_VERSION_MSG:
          messages.osVersion = data.toString('utf-8')
          break
        case SMMS_APP_VERSION_MSG:
          messages.appVersion = data.toString('utf-8')
          break
}

      offset += 1 + lengthLength + length
    }

    return messages
  }
}

module.exports = AdapterBase
