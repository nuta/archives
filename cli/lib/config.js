const fs = require('fs')
const path = require('path')
const { mkdirp } = require('hyperutils')

const CONFIG_DIR = process.env.CONFIG_DIR || `${process.env.HOME}/.makestack`

function getConfigPath(name) {
  return `${CONFIG_DIR}/${name}.json`
}

function load(name) {
  return JSON.parse(fs.readFileSync(getConfigPath(name)))
}

function save(name, data) {
  mkdirp(path.dirname(getConfigPath(name)))
  fs.writeFileSync(getConfigPath(name), JSON.stringify(data))
}

function loadCredentials() {
  return load('credentials')
}

function saveCredentials(data) {
  return save('credentials', data)
}

function loadMocks() {
  return load('mocks')
}

function updateMocks(data) {
  const mocks = (fs.existsSync(getConfigPath('mocks'))) ? loadMocks() : {}
  save('mocks', Object.assign(mocks, data))
}

function saveMocks(data) {
  return save('mocks', data)
}

module.exports = {
  loadCredentials,
  saveCredentials,
  loadMocks,
  saveMocks,
  updateMocks
}
