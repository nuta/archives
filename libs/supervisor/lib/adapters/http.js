const fetch = require('node-fetch')
const AdapterBase = require('./base')

class HTTPAdapter extends AdapterBase {
  constructor(deviceId, deviceSecret, serverURL) {
    super()

    this.serverURL = serverURL
    this.deviceId = deviceId
    this.deviceSecret = deviceSecret
  }

  computeHMAC(timestamp, body) {
    const crypto = require('crypto')
    const hash = crypto.createHash('sha256')
    const hmac = crypto.createHmac('sha256', Buffer.from(this.deviceSecret))

    hash.update(body)

    hmac.update(timestamp)
    hmac.update('\n')
    hmac.update(hash.digest('hex'))

    return hmac.digest('hex')
  }

  verifyHMAC(authorizationHeader, body) {
    const [type, timestamp, hmac] = authorizationHeader.split(' ')

    if (type !== 'SMMS') {
      console.error('unsupported Authorization header')
      return false
    }

    if ((new Date() - (new Date(timestamp))) > 5 * 60 * 1000 /* msec */) {
      console.error('too old timestamp')
      return false
    }

    if (hmac !== this.computeHMAC(timestamp, body)) {
      console.error('invalid hmac')
      return false
    }

    return true
  }

  send(messages) {
    const body = this.serialize(messages)
    const timestamp = (new Date()).toISOString()
    const hmac = this.computeHMAC(timestamp, body)
    const headers = {
      authorization: `SMMS ${timestamp} ${hmac}`
    }

    fetch(`${this.serverURL}/api/v1/smms`, {
      method: 'POST',
      body,
      headers
    }).then(response => {
      if (response.status === 200) {
        response.buffer().then(buffer => {
          if (!this.verifyHMAC(response.headers.get('Authorization'), buffer)) {
            console.error('server returned invalid timestamp or HMAC, ignoring...')
            return
          }

          this.onReceiveCallback(this.deserialize(buffer))
        })
      } else {
        console.error('server returned error:', response)
      }
    })
  }

  getAppImage(version) {
    return new Promise((resolve, reject) => {
      let appImageserverURL = `${this.serverURL}/api/v1/images/app/${this.deviceId}/${version}`

      fetch(appImageserverURL).then(r => {
        r.buffer().then(resolve).catch(reject)
      })
    })
  }

  getOSImage(deviceType, version) {
    return new Promise((resolve, reject) => {
      let url = `${this.serverURL}/api/v1/images/os/linux/${version}/${deviceType}`

      fetch(url).then(r => {
        r.buffer().then(resolve).catch(reject)
      })
    })
  }
}

module.exports = HTTPAdapter
