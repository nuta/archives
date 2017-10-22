const fs = require('fs')
const path = require('path')
const JSZip = require('jszip')
const fetch = require('node-fetch')
const { find } = require('hyperutils')
const api = require('../api')
const logger = require('../logger')
const { loadAppJSON } = require('../appdir')
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

async function mergeZipFiles(pluginName, destZip, srcZip) {
  for (const filepath in srcZip.files) {
    destZip.file(path.join('node_modules', pluginName, filepath),
      srcZip.files[filepath].async('arraybuffer'))
  }

  return destZip
}

module.exports = async (args, opts) => {
  const appName = loadAppJSON(opts.appDir).name
  let runtime = 'app-runtime'
  let plugins = [runtime]
  let zip = new JSZip()

  // Populate plugin files.
  for (let i = 0; i < plugins.length; i++) {
    const pluginName = plugins[i]

    logger.progress(`downloading \`${pluginName}'`)
    const pluginZip = await downloadPlugin(pluginName)

    logger.progress(`extracting \`${pluginName}'`)
    zip = await mergeZipFiles(pluginName, zip, await (new JSZip()).loadAsync(pluginZip))
  }

  // Copy start.js to the top level.
  logger.progress(`copying start.js from \`${runtime}'`)
  const startJsRelPath = `node_modules/${runtime}/start.js`
  zip.file('start.js', zip.files[startJsRelPath].async('arraybuffer'))

  // Copy app files.
  logger.progress(`copying files from \`${appName}'`)
  let files = find(opts.appDir)
  for (let i = 0; i < files.length; i++) {
    logger.debug(`adding \`${files[i]}'`)
    zip.file(files[i], fs.createReadStream(path.join(opts.appDir, files[i])))
  }

  logger.progress(`generating zip file`)
  const data = Buffer.from(await zip.generateAsync({ type: 'arraybuffer' }))

  logger.progress(`deploying`)
  await api.deploy(appName, data)
}
