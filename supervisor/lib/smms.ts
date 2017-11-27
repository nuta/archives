import * as assert from 'assert';
import { computeHMAC } from './hmac';

const SMMS_VERSION = 1
const SMMS_HMAC_MSG = 0x06
const SMMS_TIMESTAMP_MSG = 0x07
const SMMS_DEVICE_ID_MSG = 0x0a
const SMMS_DEVICE_INFO_MSG = 0x0b
const SMMS_LOG_MSG = 0x0c
const SMMS_OS_VERSION_MSG = 0x10
const SMMS_APP_VERSION_MSG = 0x11
const SMMS_OS_IMAGE_HMAC_MSG = 0x12
const SMMS_APP_IMAGE_HMAC_MSG = 0x13
const SMMS_STORE_MSG = 0x20

export function generateVariableLength(buf: Buffer) {
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

export function parseVariableLength(buf: Buffer) {
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

export function generateMessage(type: number, payload: any) {
  const buf = Buffer.from(payload)
  const lenbuf = generateVariableLength(buf)
  const msg = Buffer.alloc(1 + buf.length + 1)
  msg.writeUInt8(type, 0)
  lenbuf.copy(msg, 1) // size
  buf.copy(msg, 1 + lenbuf.length)

  return msg
}

export function serialize(messages, { includeDeviceId, includeHMAC, deviceSecret }) {
  let payload = Buffer.alloc(0)

  if (includeDeviceId && messages.deviceId) {
    const deviceIdMsg = generateMessage(SMMS_DEVICE_ID_MSG, messages.deviceId)
    payload = Buffer.concat([payload, deviceIdMsg])
  }

  if (messages.state) {
    const states = { booting: 1, ready: 2, running: 3 }

    if (!states[messages.state]) {
      throw new Error(`Invalid device state: \`${messages.state}'`)
    }

    const data = [states[messages.state]]
    const deviceInfoMsg = generateMessage(SMMS_DEVICE_INFO_MSG, data)

    payload = Buffer.concat([payload, deviceInfoMsg])
  }

  if (messages.log) {
    const logMsg = generateMessage(SMMS_LOG_MSG, messages.log)
    payload = Buffer.concat([payload, logMsg])
  }

  if (messages.osVersion) {
    const osVersionMsg = generateMessage(SMMS_OS_VERSION_MSG, messages.osVersion)
    payload = Buffer.concat([payload, osVersionMsg])
  }

  if (messages.appVersion) {
    const appVersionMsg = generateMessage(SMMS_APP_VERSION_MSG, messages.appVersion)
    payload = Buffer.concat([payload, appVersionMsg])
  }

  let header = Buffer.alloc(1)
  header.writeUInt8(SMMS_VERSION << 4, 0)

  if (includeHMAC) {
    const timestamp = (new Date()).toISOString()
    const timestampMsg = generateMessage(SMMS_TIMESTAMP_MSG, timestamp)
    payload = Buffer.concat([payload, timestampMsg])

    const hmacMsgLength = 1 + 1 + 64 // type, length, sha256sum
    const dummy = Buffer.concat([payload, Buffer.alloc(hmacMsgLength)])
    header = Buffer.concat([header, generateVariableLength(dummy)])

    const hmac = computeHMAC(deviceSecret, Buffer.concat([header, payload]))
    const hmacMsg = generateMessage(SMMS_HMAC_MSG, hmac)

    assert.equal(hmacMsgLength, hmacMsg.length)

    payload = Buffer.concat([payload, hmacMsg])
  } else {
    header = Buffer.concat([header, generateVariableLength(payload)])
  }

  return Buffer.concat([header, payload])
}

export function deserialize(payload: Buffer) {
  let version = payload.readUInt8(0)
  if (version >> 4 !== SMMS_VERSION) {
    throw new Error('unsupported smms version')
  }

  const [totalLength, totalLengthLength] = parseVariableLength(payload.slice(1))
  const headerLength = 1 + totalLengthLength
  let messages: any = {}
  let offset = headerLength
  let hmacProtectedEnd = null
  while (offset < headerLength + totalLength) {
    if (hmacProtectedEnd) {
      throw new Error('invalid payload: hmac message must be the last one')
    }

    const type = payload[offset]
    const [length, lengthLength] = parseVariableLength(payload.slice(offset + 1))
    const dataOffset = offset + 1 + lengthLength
    const data = payload.slice(dataOffset, dataOffset + length)

    switch (type) {
      case SMMS_STORE_MSG: {
        const keyLengthOffset = dataOffset
        const [keyLength, keyLengthLength] = parseVariableLength(payload.slice(keyLengthOffset))
        const keyOffset = keyLengthOffset + keyLengthLength
        const valueOffset = keyOffset + keyLength
        const valueLength = length - (valueOffset - keyLengthOffset)
        const key = payload.slice(keyOffset, keyOffset + keyLength)
        const value = payload.slice(valueOffset, valueOffset + valueLength)

        if (!('stores' in messages)) {
          messages.stores = {}
        }

        messages.stores[key.toString('utf-8')] = value.toString('utf-8')
        break
      }
      case SMMS_OS_VERSION_MSG:
        messages.osVersion = data.toString('utf-8')
        break
      case SMMS_APP_VERSION_MSG:
        messages.appVersion = data.toString('utf-8')
        break
      case SMMS_OS_IMAGE_HMAC_MSG:
        messages.osImageHMAC = data.toString('utf-8')
        break
      case SMMS_APP_IMAGE_HMAC_MSG:
        messages.appImageHMAC = data.toString('utf-8')
        break
      case SMMS_HMAC_MSG:
        messages.hmac = data.toString('utf-8')
        hmacProtectedEnd = offset
        break
      case SMMS_TIMESTAMP_MSG:
        messages.timestamp = data.toString('utf-8')
        break
    }
    offset += 1 + lengthLength + length
  }

  return [messages, hmacProtectedEnd]
}
