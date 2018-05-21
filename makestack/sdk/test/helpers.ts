import { spawnSync } from 'child_process';
import * as fs from 'fs';
import * as path from 'path';
import * as nock from 'nock';
import * as mockfs from 'mock-fs';
import * as JSZip from 'jszip';

const CONFIG_DIR = '/user/.makestack'
process.env.CONFIG_DIR = CONFIG_DIR

const url = 'http://test-server'
const username = 'test-user'
const email = 'test-user@example.com'
const uid = '123456789'
const accessToken = 'qwerty'
const accessTokenSecret = 'asdfg'

export function createApiRequest() {
    return nock(`${url}/api/v1`);
}

export const CONFIG_FILES = {
    [path.join(CONFIG_DIR, 'credentials.json')]: '',
    '/user': {}
}

export function login() {
    fs.writeFileSync(path.join(CONFIG_DIR, 'credentials.json'), JSON.stringify({
        'access-token': accessToken,
        'access-token-secret': accessTokenSecret,
        uid,
        email,
        username,
        url
    }))
}

export function createApp(name: string = 'app'): string {
    const appDir = `/user/${name}`
    fs.mkdirSync(appDir)
    fs.writeFileSync(path.join(appDir, 'package.json'), `{ "name": "${name}" }`)
    fs.writeFileSync(path.join(appDir, 'app.js'), 'print("Hi!")')
    return appDir
}

export async function createZip(files: { [path: string]: string }): Promise<Buffer> {
    const zip = new JSZip()
    for (const filepath in files) {
        zip.file(filepath, files[filepath])
    }

    return zip.generateAsync({ type: 'nodebuffer' }) as Promise<Buffer>
}
