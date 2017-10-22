const fs = require('fs')
const path = require('path')
const JSZip = require('jszip')
const fetch = require('node-fetch')
const { find } = require('hyperutils')
const api = require('../api')
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

  console.log(`==> downloading \`${name}'`)
  return (await fetch(url)).buffer()
}

async function mergeZipFiles(pluginName, destZip, srcZip) {
  for (const filepath in srcZip.files) {
    destZip.file(path.join('node_modules', pluginName, filepath),
      srcZip.files[filepath].async('arraybuffer'))
  }

  return destZip
}

module.exports = async (args, opts, logger) => {
  const appDir = path.join(process.cwd(), 'app')
  const appName = 'deploy-test'
  let runtime = 'app-runtime'
  let plugins = [runtime]
  let zip = new JSZip()

  // Populate plugin files.
  for (let i = 0; i < plugins.length; i++) {
    const pluginName = plugins[i]
    const pluginZip = await downloadPlugin(pluginName)
    zip = await mergeZipFiles(pluginName, zip, await (new JSZip()).loadAsync(pluginZip))
  }

  // Copy start.js to the top level.
  const startJsRelPath = 'node_modules/app-runtime/start.js'
  zip.file('start.js', zip.files[startJsRelPath].async('arraybuffer'))

  // Copy app files.
  let files = find(appDir)
  for (let i = 0; i < files.length; i++) {
    zip.file(files[i], fs.createReadStream(path.join(appDir, files[i])))
  }

  const data = Buffer.from(await zip.generateAsync({ type: 'arraybuffer' }))
  await api.deploy(appName, data)
}
