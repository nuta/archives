import * as assert from 'power-assert';
import { spawnSync } from 'child_process';
import * as fs from 'fs';
import * as path from 'path';
import * as nock from 'nock';
import * as mockfs from 'mock-fs';

const CONFIG_DIR = '/user/.makestack'
process.env.CONFIG_DIR = CONFIG_DIR
const makestack = require('../..')

const server = 'http://test-server'
const username = 'test-user'
const email = 'test-user@example.com'
const password = '12345678'
const uid = '123456789'
const accessToken = 'qwerty'
const accessTokenSecret = 'asdfg'

nock(server)
    .post('/api/v1/auth/sign_in', { username, password })
    .reply(200, {
        data: { username, email }
    }, {
        uid,
        'access-token': accessToken,
        'access-token-secret': accessTokenSecret
    })

function loadJSON(filepath) {
    return JSON.parse(fs.readFileSync(filepath, { encoding: 'utf-8' }))
}

describe('login', function() {
    beforeEach(function() {
        this.appDir = '/app'
        this.credentialsJSON = path.join(CONFIG_DIR, 'credentials.json')
        mockfs({
            [this.appDir]: {}
        })
    })

    afterEach(function() {
        mockfs.restore()
    })

    it('creates .credentials.json', async function() {
        await makestack.login(server, username, password)
        assert.ok(fs.existsSync(this.credentialsJSON))
        assert.deepEqual(loadJSON(this.credentialsJSON), {
            uid,
            username,
            email,
            url: server,
            'access-token': accessToken,
            'access-token-secret': accessTokenSecret
        })
    })
})
