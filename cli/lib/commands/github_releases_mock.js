const fs = require('fs')
const os = require('os')
const path = require('path')
const express = require('express')
const buildPlugin = require('../plugin_builder')

const DUMMY_LINUX_OS_IMAGE = `🚀🚀🚀 MOCK DEVICE OS IMAGE DATA 🛫🛫🛫'
os_version="_____REPLACE_ME_MAKESTACK_CONFIG_OS_VERSION_____"
device_type="_____REPLACE_ME_MAKESTACK_CONFIG_DEVICE_TYPE_____"
device_id="_____REPLACE_ME_MAKESTACK_CONFIG_DEVICE_ID_____"
device_secret="_____REPLACE_ME_MAKESTACK_CONFIG_DEVICE_SECRET_____"
adapter="_____REPLACE_ME_MAKESTACK_CONFIG_ADAPTER_____"
server_url="_____REPLACE_ME_MAKESTACK_CONFIG_SERVER_URL_abcdefghijklmnopqrstuvwxyz1234567890_____"
`

module.exports = (args, opts, logger) => {
  const server = express()

  server.get('/repos/plugins/app-runtime.plugin.zip', (req, res) => {
    const appRuntimePath = path.resolve(__dirname, '../../../libs/app-runtime')
    const destPath = path.resolve(os.tmpdir(), 'app-runtime.plugin.zip')
    buildPlugin(appRuntimePath, destPath)
    res.send(fs.readFileSync(destPath))
  })

  server.get('/repos/os/:type.img', (req, res) => {
    const images = {
      x86_64() { return fs.readFileSync('linux/build/x86_64/bootfs/kernel7.img') },
      raspberypi3() { return fs.readFileSync('linux/build/raspberrypi3/bootfs/vmlinuz') },
      mock() { return DUMMY_LINUX_OS_IMAGE }
    }

    res.send(images[req.params.type]())
  })

  server.get('/repos/:org/:repo/releases/latest', (req, res) => {
    let json
    if (req.params.org === 'seiyanuta' && req.params.repo === 'makestack') {
      json = {
        tag_name: 'v12345.0.0',
        assets: [{
          name: 'app-runtime.plugin.zip',
          browser_download_url: `http://localhost:8080/repos/plugins/app-runtime.plugin.zip`
        }, {
          name: 'makestack-linux-mock-v12345.0.0.img',
          browser_download_url: `http://localhost:8080/repos/os/mock`
        }]
      }
    } else {
      json = {
        tag_name: 'v12345.0.0',
        assets: [{
          name: `${req.params.repo}.plugin.zip`,
          browser_download_url: `http://localhost:8080/repos/plugins/${req.params.repo}.plugin.zip`
        }]
      }
    }

    res.send(json)
  })

  const port = 8100
  server.listen(port, () => {
    logger.info(`listening on ${port}`)
  })
}
