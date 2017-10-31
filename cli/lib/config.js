const fs = require('fs')
const path = require('path')
const { mkdirp } = require('hyperutils')

const CONFIG_DIR = process.env.CONFIG_DIR || `${process.env.HOME}/.makestack`

function load(name) {
  return JSON.parse(fs.readFileSync(`${CONFIG_DIR}/${name}.json`))
}

function save(name, data) {
  const filepath = `${CONFIG_DIR}/${name}.json`
  mkdirp(path.dirname(filepath))
  fs.writeFileSync(filepath, JSON.stringify(data))
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
  save('mocks', Object.assign(loadMocks(), data))
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
