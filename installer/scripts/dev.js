'use strict'

process.env.NODE_ENV = 'development'

const chalk = require('chalk')
const electron = require('electron')
const path = require('path')
const { spawn } = require('child_process')
const webpack = require('webpack')
const WebpackDevServer = require('webpack-dev-server')
const webpackHotMiddleware = require('webpack-hot-middleware')

const mainConfig = require('../webpack/webpack.main.config')
const rendererConfig = require('../webpack/webpack.renderer.config')


let electronProcess = null
let manualRestart = false
let hotMiddleware

function error(component, data) {
  print(component, data, chalk.red.bold)
}

function progress(component, data) {
  print(component, data)
}

function printWebpackStats(component, data) {
  print(component, data.toString({ colors: true, chunks: false }))
}

function print(component, data, color) {
  let colors = {
    renderer: chalk.yellow,
    main: chalk.magenta,
    electron: chalk.blue
  }

  if (color === undefined)
    color = colors[component] || chalk.grey

  let messages;
  if (typeof data === 'object') {
    data = data.toString()
  }
  
  data.split("\n").forEach(line => {
    console.log(`${color((component + ':').padStart(10))} ${line}`)
  });
}

function startRenderer () {
  return new Promise((resolve, reject) => {
    rendererConfig.entry.renderer = [path.join(__dirname, 'dev-client')].concat(rendererConfig.entry.renderer)

    const compiler = webpack(rendererConfig)
    hotMiddleware = webpackHotMiddleware(compiler, { 
      log: false, 
      heartbeat: 2500 
    })

    compiler.plugin('compilation', compilation => {
      compilation.plugin('html-webpack-plugin-after-emit', (data, cb) => {
        hotMiddleware.publish({ action: 'reload' })
        cb()
      })
    })

    compiler.plugin('done', stats => {
      printWebpackStats('renderer', stats)
    })

    const server = new WebpackDevServer(
      compiler,
      {
        contentBase: path.join(__dirname, '../'),
        quiet: true,
        before(app, ctx) {
          app.use(hotMiddleware)
          ctx.middleware.waitUntilValid(() => {
            resolve()
          })
        }
      }
    )

    server.listen(9080)
  })
}

function startMain () {
  return new Promise((resolve, reject) => {
    mainConfig.entry.main = [path.join(__dirname, '../main/index.js')];

    const compiler = webpack(mainConfig)

    compiler.plugin('watch-run', (compilation, done) => {
      progress('main', 'compling...')      
      hotMiddleware.publish({ action: 'compiling' })
      done()
    })

    compiler.watch({}, (err, stats) => {
      if (err) {
        console.log(err)
        return
      }

      printWebpackStats('main', stats)
      
      if (electronProcess && electronProcess.kill) {
        manualRestart = true
        process.kill(electronProcess.pid)
        electronProcess = null
        startElectron()

        setTimeout(() => {
          manualRestart = false
        }, 5000)
      }

      resolve()
    })
  })
}

function startElectron () {
  electronProcess = spawn(electron, [path.join(__dirname, '../dist/electron/main.js')])

  electronProcess.stdout.on('data', data => {
    progress('electron', data)
  })
  electronProcess.stderr.on('data', data => {
    error('electron', data)
  })

  electronProcess.on('close', () => {
    if (!manualRestart) process.exit()
  })
}

function init () {
  Promise.all([startRenderer(), startMain()])
    .then(() => {
      startElectron()
    })
    .catch(err => {
      console.error(err)
    })
}

init()