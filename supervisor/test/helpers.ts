import * as crypto from 'crypto';
import * as fs from 'fs';
import * as os from 'os';
import * as path from 'path';
import * as nock from 'nock';
import * as sinon from 'sinon';
import * as JSZip from 'jszip';
import * as smms from '../lib/smms';
import { PayloadMessages } from '../lib/types';

const { Supervisor } = require('..');

const APP_DIR = path.resolve(os.tmpdir(), 'makestack-supervisor-test-app')
const DEVICE_ID = 'Uz3GDcfqQ0axGQ70p5x30asCzjT0bLOumk-YdeG0'
const DEVICE_SECRET = 'MsK91P2I6kVfFKdHXPbe.UEySJuwZuuNiLECQqnh'
const SERVER_URL = 'http://test-server'
const OS_VERSION = 'c'

async function createZip(files: { [key: string]: string }): Promise<Buffer> {
    const zip = new JSZip()
    for (const filepath in files) {
        zip.file(filepath, files[filepath])
    }

    return zip.generateAsync({ type: 'nodebuffer' }) as Promise<Buffer>
}

export function computeImageHMAC(image: Buffer): string {
    const shasum = crypto.createHash('sha256').update(image).digest('hex')
    return crypto.createHmac('sha256', DEVICE_SECRET).update(shasum).digest('hex')
}

export async function createAppImage(files: { [key: string]: string }): Promise<{ image: Buffer, hmac: string }> {
    const appZip = await createZip({
        '/start.js': 'process.send({ type: "log", body: "I am your app, Luke." })'
    })

    return {
        image: appZip,
        hmac: computeImageHMAC(appZip)
    }
}

export function createHeartbeatResponse(messages: PayloadMessages) {
    const serializeOptions = {
        includeDeviceId: true,
        includeHMAC: true,
        deviceSecret: DEVICE_SECRET
    }

    return nock(SERVER_URL)
         .post('/api/v1/smms', () => true)
         .reply(200, smms.serialize(messages, serializeOptions))
}

export function createAppImageResponse(version: string, image: Buffer) {
    return nock(SERVER_URL)
        .get(`/api/v1/images/app/${DEVICE_ID}/${version}`)
        .reply(200, image)
}

export function createOSImageResponse(osType: string, deviceType: string, version: string, image: Buffer) {
    return nock(SERVER_URL)
        .get(`/api/v1/images/os/${DEVICE_ID}/${version}/${osType}/${deviceType}`)
        .reply(200, image)
}

export function prepareAppDir() {
    if (!fs.existsSync(APP_DIR)) {
        fs.mkdirSync(APP_DIR)
    }
}

export function createSupervisor() {
    return new Supervisor({
        mode: 'test',
        appDir: APP_DIR,
        adapter: {
            name: 'http',
            url: SERVER_URL
        },
        osType: 'mock',
        deviceType: 'raspberrypi3',
        deviceId: DEVICE_ID,
        deviceSecret: DEVICE_SECRET,
        osVersion: OS_VERSION,
        heartbeatInterval: 60,
        runtimeModulePath: path.resolve(__dirname, '../../runtime')
    })
}
