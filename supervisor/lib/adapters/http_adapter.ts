import http = require('http');
import https = require('https');
import { parse as parseURL } from 'url';
import { AdapterBase } from './adapter_base';
import * as logger from '../logger';

function request(method: string, url: string, body?: Buffer): Promise<Buffer> {
  return new Promise((resolve, reject) => {
    const send: typeof http.request = (url.startsWith('https://') ? https : http).request;

    const options = Object.assign({}, parseURL(url), { method })
    const req = send(options)

    if (body) {
      req.write(body)
    }

    req.end()

    let responseBody = Buffer.alloc(0)

    req.on('error', err => {
      reject(new Error(`failed to perform a http request: ${err.message}`))
    })

    req.once('response', (res: http.IncomingMessage) => {
      res.on('data', (chunk: Buffer) => {
        responseBody = Buffer.concat([responseBody, chunk])
      })

      res.on('end', () => {
        if (res.statusCode === 200) {
          resolve(responseBody)
        } else {
          reject(new Error(`server returned ${res.statusMessage}`))
        }
      })
    })
  })
}

export default class HTTPAdapter extends AdapterBase {
  osType: string;
  deviceType: string;
  deviceId: string;
  serverURL: string;

  constructor(osType, deviceType, deviceId, serverURL) {
    super()

    this.osType = osType
    this.deviceType = deviceType
    this.deviceId = deviceId
    this.serverURL = serverURL
  }

  connect() {
    // Nothing to do.
  }

  send(payload) {
    return request('POST', `${this.serverURL}/api/v1/smms`, payload)
      .then(this.onReceiveCallback)
      .catch(logger.error)
  }

  getAppImage(version) {
    const url = `${this.serverURL}/api/v1/images/app/${this.deviceId}/${version}`
    return request('GET', url)
  }

  getOSImage(version) {
    const url = `${this.serverURL}/api/v1/images/os/${this.deviceId}/${version}/${this.osType}/${this.deviceType}`
    return request('GET', url)
  }
}
