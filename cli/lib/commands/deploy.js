const fs = require('fs')
const path = require('path')
const JSZip = require('jszip')
const fetch = require('node-fetch')
const { find } = require('hyperutils')
const api = require('../api')

function fetchLatestGitHubRelease(name) {
  // TODO: Error handling
  return new Promise((resolve, reject) => {
    fetch(`https://api.github.com/repos/${name}/releases/latest`).then(resp => {
      return resp.json()
    }).then(json => {
      resolve(json.assets || [])
    })
  })
}

async function getPluginLatestReleaseURL(name) {
  console.log(`==> searching for \`${name}'`)

  const assets = await fetchLatestGitHubRelease()
  return assets[0].browser_download_url
}

async function downloadPlugin(name) {
  let url
  if (name.match(/^[a-zA-Z0-9\-\_]+$/)) {
    // Builtin plugins
    if (process.env.FETCH_BUILTIN_PLUGINS_FROM_REPO) {
      // Fetch the plugin from this repository.
      let zip = JSZip()

      if (name != 'app-runtime') {
        throw new Error(`unknown plugin: \`${name}'`)
      }

      // XXX:
      const pluginDir = path.resolve(__dirname, '../../../libs/app-runtime')
      let files = find(pluginDir)

      for (let i = 0; i < files.length; i++) {
        zip.file(path.join(name, files[i]), fs.createReadStream(path.join(pluginDir, files[i])))
      }

      return zip.generateAsync({ type: 'arraybuffer' })
    } else {
      // Download from GitHub.
      const assets = await fetchLatestGitHubRelease()
      url = assets.filter(asset => asset.name.indexOf(name) !== -1)[0]
      if (!url) {
        throw new Error(`unknown plugin: \`${name}'`)
      }
    }
  } else if (name.match(/^[a-zA-Z0-9\-\_]+\/[a-zA-Z0-9\-\_]+$/)) {
    // Plugins on GitHub
    url = await getPluginLatestReleaseURL(name)
  } else {
    throw new Error(`invalid plugin name: \`${name}'`)
  }

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
