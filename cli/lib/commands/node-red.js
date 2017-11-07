const fs = require('fs')
const os = require('os')
const path = require('path')
const chalk = require('chalk')
const express = require('express')
const bodyParser = require('body-parser')
const proxy = require('http-proxy-middleware')
const { spawn, spawnSync } = require('child_process')
const { mkdirp } = require('hyperutils')
const { deploy } = require('../deploy')
const { loadAppYAML } = require('../appdir')

const NODE_RED_DIR = path.resolve(os.homedir(), '.makestack/node-red')
const NODE_RED_URL = 'https://github.com/node-red/node-red/archive/0.17.5.tar.gz'
const NODE_RED_PORT = 1880
const NODE_RED_SETTINGS = `
module.exports = {
  uiPort: ${NODE_RED_PORT},
  uiHost: "127.0.0.1",
  debugMaxLength: 1000,
  logging: {
    console: {
      level: "info", // trace, debug, info, warn, error, or fatal
      metrics: false,
      audit: false
    }
  }
  editorTheme: {
    page: {
      title: "Node-RED for MakeStack",
    },
    header: {
      title: "Node-RED for MakeStack",
    },
    deployButton: {
      type:  "simple",
      label: "Deploy"
    }
  }
}
`

function installNodeRED() {
  if (!fs.existsSync(path.join(NODE_RED_DIR, 'red.js'))) {
    mkdirp(NODE_RED_DIR)
    spawnSync('curl', ['-LO', NODE_RED_URL], { cwd: NODE_RED_DIR })
    spawnSync('tar',
      ['xf', path.basename(NODE_RED_URL), '--strip-components', '1'],
      { stdio: 'inherit', cwd: NODE_RED_DIR })
    spawnSync('npm', ['install'], { stdio: 'inherit', cwd: NODE_RED_DIR })
    spawnSync('npx', ['grunt', 'build'], { stdio: 'inherit', cwd: NODE_RED_DIR })
  }

  fs.writeFileSync(path.join(NODE_RED_DIR, 'settings.js'), NODE_RED_SETTINGS)
}

function spawnNodeRED() {
  const cb = spawn('node', ['red'], { cwd: NODE_RED_DIR })

  cb.stdout.setEncoding('utf8')
  cb.stderr.setEncoding('utf8')
  cb.stdout.on('data', data => console.log(chalk.blue(`Node-RED: ${data.trimRight()}`)))
  cb.stderr.on('data', data => console.log(chalk.red(`Node-RED: ${data.trimRight()}`)))
  cb.on('close', () => {
    console.error(chalk.bold.red('Node-RED exited unexpectedly.'))
    process.exit(1)
  })

  process.on('exit', () => {
    cb.kill()
  })
}

function loadFlows(flowsFilePath) {
  const app = JSON.parse(fs.readFileSync(flowsFilePath, { encoding: 'utf-8' }) || '{}')
  return { flows: app.flows }
}

function transpile(flows) {
  return 'console.log("HELO!")'
}

async function deployFlows(appDir, flowsFilePath, body) {
  const nodeREDJSON = {
    flows: body.flows
  }

  fs.writeFileSync(flowsFilePath, JSON.stringify(nodeREDJSON, null, 2))
  const script = transpile(body.flows)
  await deploy(loadAppYAML(appDir), [{ path: 'app.js', body: script }])
}

function spawnProxyServer(port, appDir, flowsFilePath) {
  const server = express()
  server.use(bodyParser.json())

  server.get('/flows', (req, res) => {
    /* Return saved Node-RED app flows. */
    res.send(loadFlows(flowsFilePath))
  })

  server.post('/flows', (req, res) => {
    /* Save aand Deploy the app. */
    try {
      deployFlows(appDir, flowsFilePath, req.body)
        .then(r => res.status(200).send(r))
        .catch(e => {
          console.error(e)
          res.status(500).send(e)
        })
    } catch (e) {
      console.error(e)
      res.status(500).send(e)
    }
  })

  server.use('/', proxy({
    target: `http://localhost:${NODE_RED_PORT}`,
    changeOrigin: true,
    ws: true
  }))

  server.listen(port, () => {
    console.log(chalk.bold.green(`Open http://localhost:${port} on your web browser!`))
  })
}

module.exports = (args, opts, logger) => {
  const flowsFilePath = path.resolve(opts.appDir, 'node-red.json')

  logger.info('==> Setting up Node-RED')
  installNodeRED()
  logger.info('==> Starting Node-RED')
  spawnNodeRED()
  logger.info('==> Starting a proxy server')
  spawnProxyServer(opts.port, opts.appDir, flowsFilePath)
}
