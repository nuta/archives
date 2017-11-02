const assert = require('power-assert')
const { spawnSync } = require('child_process')
const fs = require('fs')
const path = require('path')
const nock = require('nock')

const TMP_CONFIG_DIR = path.resolve(__dirname, 'tmp/config')
process.env.CONFIG_DIR = TMP_CONFIG_DIR
const makestack = require('..')

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
  return JSON.parse(fs.readFileSync(filepath))
}

describe('login', () => {
  beforeEach(() => {
    spawnSync(`rm -r ${TMP_CONFIG_DIR}`)
  })

  it('creates .credentials.json', async () => {
    const credentialsJSON = path.join(TMP_CONFIG_DIR, 'credentials.json')

    await makestack.login(server, username, password)
    assert.ok(fs.existsSync(credentialsJSON))
    assert.deepEqual(loadJSON(credentialsJSON), {
      uid,
      username,
      email,
      url: server,
      'access-token': accessToken,
      'access-token-secret': accessTokenSecret
    })
  })
})
