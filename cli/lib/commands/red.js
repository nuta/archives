const crypto = require('crypto')
const fs = require('fs')
const os = require('os')
const path = require('path')
const chalk = require('chalk')
const express = require('express')
const bodyParser = require('body-parser')
const proxy = require('http-proxy-middleware')
const supervisor = require('supervisor')
const { spawn, spawnSync } = require('child_process')
const { mkdirp } = require('hyperutils')
const { deploy } = require('../deploy')
const { loadAppYAML } = require('../appdir')

const NODE_RED_USER_DIR = path.resolve(os.homedir(), '.makestack/node-red')
const NODE_RED_DIR = path.resolve(os.homedir(), '.makestack/node-red/node-red')
const NODE_RED_URL = 'https://github.com/node-red/node-red/archive/0.17.5.tar.gz'
const NODE_RED_PORT = 1880
const NODE_RED_SETTINGS = `
module.exports = {
  uiPort: ${NODE_RED_PORT},
  uiHost: "127.0.0.1",
  debugMaxLength: 1000,
  nodesDir: "${path.resolve(NODE_RED_USER_DIR, 'nodes')}",
  coreNodesDir: "${path.resolve(__dirname, '../red/nodes')}",
  logging: {
    console: {
      level: "info", // trace, debug, info, warn, error, or fatal
      metrics: false,
      audit: false
    }
  },
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
    mkdirp(NODE_RED_USER_DIR)
    spawnSync('curl', ['-LO', NODE_RED_URL], { cwd: NODE_RED_DIR })
    spawnSync('tar',
      ['xf', path.basename(NODE_RED_URL), '--strip-components', '1'],
      { stdio: 'inherit', cwd: NODE_RED_DIR })
    spawnSync('npm', ['install'], { stdio: 'inherit', cwd: NODE_RED_DIR })
    spawnSync('npx', ['grunt', 'build'], { stdio: 'inherit', cwd: NODE_RED_DIR })
  }

  fs.writeFileSync(path.join(NODE_RED_USER_DIR, 'settings.js'), NODE_RED_SETTINGS)
}

function spawnNodeRED() {
  const cb = spawn('node', ['red', '-u', NODE_RED_USER_DIR], { cwd: NODE_RED_DIR })

  cb.stdout.setEncoding('utf8')
  cb.stderr.setEncoding('utf8')
  cb.stdout.on('data', data => console.log(chalk.blue(`Node-RED: ${data.trimRight()}`)))
  cb.stderr.on('data', data => console.log(chalk.red(`Node-RED: ${data.trimRight()}`)))
  cb.on('close', () => {
    console.error(chalk.bold.red('Node-RED exited unexpectedly.'))
    process.exit(1)
  })

  process.on('SIGTERM', () => process.exit(1))
  process.on('SIGINT', () => process.exit(1))
  process.on('exit', () => {
    cb.kill()
  })
}

function loadFlows(nodeRedJSON) {
  const app = JSON.parse(fs.readFileSync(nodeRedJSON, { encoding: 'utf-8' }) || '{}')
  return { flows: app.flows }
}

function loadTranspilers() {
  const transiplerNames = ['inject', 'log']
  const transpilers = []
  for (const name of transiplerNames) {
    transpilers[name] = require(`../red/transpilers/${name}`)
  }

  return transpilers
}

function generateNodeId(id) {
  const hash = crypto.createHash('sha1').update(id).digest('hex').substring(0, 16)
  return `__${hash}__`
}

function transpile(flows) {
  const transpilers = loadTranspilers()

  console.log(require('util').inspect(flows, false, 8))

  let code = `
    const EventEmitter = require('events')
    let __nodes__ = {}

    function __next__(nodeId, outputs) {
      for (const [index, output] of outputs.entries()) {
        for (const nextNodeId of __nodes__[nodeId].outputs[index]) {
          __nodes__[nextNodeId].ev.emit('input', output)
        }
      }
    }

    function __invoke_input_nodes__() {
      for (const id in __nodes__) {
        const { type, ev } = __nodes__[id]
        if (type === 'input') {
          ev.emit('input', {})
        }
      }
    }

    function __initialize_nodes__() {
  `

  for (const flow of flows) {
    if (flow.type === 'tab') {
      continue
    }

    const nodeId = generateNodeId(flow.id)
    const { type: nodeType, code: nodeCodeContent } = transpilers[flow.type](flow)
    const outputs = JSON.stringify(flow.wires.map(wire => wire.map(generateNodeId)))

    let nodeCode = `
      const ev_${nodeId} = new EventEmitter()
      __nodes__['${nodeId}'] = {
        ev: ev_${nodeId},
        type: '${nodeType}',
        outputs: ${outputs}
      }

      ev_${nodeId}.on('input', __inputs__ => {
        const __input__ = __inputs__[0];
        ${nodeCodeContent}
      })

    `

    nodeCode = nodeCode.replace('__outputs__()', `__next__('${nodeId}')`)
    nodeCode = nodeCode.replace('__outputs__(', `__next__('${nodeId}', `)
    code += nodeCode
  }

  code += `
    }

    __initialize_nodes__()
    __invoke_input_nodes__()
  `

  console.log(code)
  return code
}

async function deployFlows(appDir, nodeRedJSON, body) {
  const nodeREDJSON = {
    flows: body.flows
  }

  fs.writeFileSync(nodeRedJSON, JSON.stringify(nodeREDJSON, null, 2))
  const script = transpile(body.flows)
  await deploy(loadAppYAML(appDir), [{ path: 'app.js', body: script }])
}

function spawnProxyServer(port, appDir, nodeRedJSON) {
  const server = express()
  server.use(bodyParser.json())

  server.get('/flows', (req, res) => {
    /* Return saved Node-RED app flows. */
    res.send(loadFlows(nodeRedJSON))
  })

  server.post('/flows', (req, res) => {
    /* Save aand Deploy the app. */
    try {
      deployFlows(appDir, nodeRedJSON, req.body)
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
  if (opts.dev) {
    const argv = process.argv.slice(1).filter(arg => arg !== '--dev')

    supervisor.run(['-w', path.resolve(__dirname, '../../lib') + ',' +
      path.resolve(NODE_RED_USER_DIR, '.makestack/node-red'), '-e', 'js,html', '--',
      ...argv])
    return
  }

  // Ensure that the directory specified by opts.appDir is a MakeStack app.
  loadAppYAML(opts.appDir)

  const nodeRedJSON = path.resolve(opts.appDir, 'red.json')
  if (!fs.existsSync(nodeRedJSON)) {
    fs.writeFileSync(nodeRedJSON, JSON.stringify({
      flows: [{ id: 'fa5ac137.92397', type: 'tab', label: 'Flow 1' }]
    }))
  }
  logger.info('==> Setting up Node-RED')
  installNodeRED()
  logger.info('==> Starting Node-RED')
  spawnNodeRED()
  logger.info('==> Starting a proxy server')
  spawnProxyServer(opts.port, opts.appDir, nodeRedJSON)
}
