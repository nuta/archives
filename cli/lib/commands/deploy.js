const fs = require('fs')
const path = require('path')
const JSZip = require('jszip')
const fetch = require('node-fetch')
const { find } = require('hyperutils')
const api = require('../api')
const logger = require('../logger')
const { loadAppYAML } = require('../appdir')
const { getLatestGitHubRelease } = require('../github_releases')

async function downloadPlugin(name) {
  let repo
  if (name.match(/^[a-zA-Z0-9\-\_]+$/)) {
    // Builtin plugins (e.g. app-runtime)
    repo = 'seiyanuta/makestack'
  } else if (name.match(/^[a-zA-Z0-9\-\_]+\/[a-zA-Z0-9\-\_]+$/)) {
    // Plugins on GitHub (e.g. octocat/temperature-sensor)
    repo = name
  } else {
    throw new Error(`invalid plugin name: \`${name}'`)
  }

  const [, url] = await getLatestGitHubRelease(repo, name, '.plugin.zip')

  return (await fetch(url)).buffer()
}

async function mergeZipFiles(basepath, destZip, srcZip) {
  for (const filepath in srcZip.files) {
    const file = srcZip.files[filepath].async('arraybuffer')
    destZip.file(path.join(basepath, filepath), file)
  }

  return destZip
}

async function downloadAndExtractPackage(name, zip, basepath) {
  logger.progress(`downloading \`${name}'`)
  const pluginZip = await downloadPlugin(name)

  logger.progress(`extracting \`${name}'`)
  zip = await mergeZipFiles(basepath, zip, await (new JSZip()).loadAsync(pluginZip))
  return zip
}

module.exports = async (args, opts) => {
  const appYAML = loadAppYAML(opts.appDir)
  const appName = appYAML.name
  let runtime = 'app-runtime'
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
  let files = find(opts.appDir)
  for (const file of files) {
    logger.debug(`adding \`${file}'`)
    zip.file(file, fs.createReadStream(path.join(opts.appDir, file)))
  }

  logger.progress(`generating zip file`)
  const data = Buffer.from(await zip.generateAsync({ type: 'arraybuffer' }))

  logger.progress(`deploying`)
  await api.deploy(appName, data)
}
