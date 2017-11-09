const fs = require('fs')
const path = require('path')
const JSZip = require('jszip')
const { find } = require('./helpers')
const api = require('./api')
const logger = require('./logger')
const { loadAppYAML } = require('./appdir')

async function mergeZipFiles(basepath, destZip, srcZip) {
  for (const filepath in srcZip.files) {
    const file = srcZip.files[filepath].async('arraybuffer')
    destZip.file(path.join(basepath, filepath), file)
  }

  return destZip
}

async function downloadAndExtractPackage(name, zip, basepath) {
  logger.progress(`downloading \`${name}'`)
  const pluginZip = await api.downloadPlugin(name)

  logger.progress(`extracting \`${name}'`)
  zip = await mergeZipFiles(basepath, zip, await (new JSZip()).loadAsync(pluginZip))
  return zip
}

async function deploy(appYAML, files) {
  const appName = appYAML.name
  let runtime = 'nodejs-runtime'
  let plugins = appYAML.plugins || []
  let zip = new JSZip()

  // Download the runtime.
  zip = await downloadAndExtractPackage(runtime, zip, `node_modules/${runtime}`)

  // Populate plugin files.
  for (const pluginName of plugins) {
    zip = await downloadAndExtractPackage(pluginName, zip, `plugins/${pluginName}`)
    zip.file(path.join(`plugins/${pluginName}/package.json`, JSON.stringify({
      private: true
    })))
  }

  // Copy start.js to the top level.
  logger.progress(`copying start.js from \`${runtime}'`)
  const startJsRelPath = `node_modules/${runtime}/start.js`
  zip.file('start.js', zip.files[startJsRelPath].async('arraybuffer'))

  // Copy app files.
  logger.progress(`copying files from \`${appName}'`)
  for (const file of files) {
    logger.debug(`adding \`${file}'`)
    zip.file(file.path, file.body)
  }

  logger.progress('generating zip file')
  const data = Buffer.from(await zip.generateAsync({ type: 'arraybuffer' }))

  logger.progress('deploying')
  await api.deploy(appName, data)
}

async function deployAppDir(appDir) {
  const appYAML = loadAppYAML(appDir)
  let files = []

  for (const path of find(appDir)) {
    files.push({
      path,
      body: fs.readFileSync(path.join(appDir, path))
    })
  }

  await deploy(appYAML, files)
}

module.exports = { deployAppDir, deploy }
