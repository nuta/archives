import * as fs from 'fs';
import * as path from 'path';
import * as JSZip from 'jszip';
import { find } from './helpers';
import { api } from './api';
import { logger } from './logger';
import { loadAppYAML } from './appdir';

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

export async function deploy(appYAML, files) {
  const appName = appYAML.name
  let runtime = 'runtime'
  let plugins = appYAML.plugins || []
  let zip = new JSZip()

  // Download the runtime.
  zip = await downloadAndExtractPackage(runtime, zip, `node_modules/@makestack/${runtime}`)

  // Populate plugin files.
  for (const pluginName of plugins) {
    zip = await downloadAndExtractPackage(pluginName, zip, `node_modules/@makestack/${pluginName}`)
    if (!zip.files[`node_modules/@makestack/${pluginName}/package.json`]) {
      zip.file(`node_modules/@makestack/${pluginName}/package.json`,
        JSON.stringify({ name: pluginName, private: true }))
    }
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
  const appImage = Buffer.from(await zip.generateAsync({
    type: 'arraybuffer',
    compression: 'DEFLATE',
    compressionOptions: {
      level: 9
    }
  }))

  logger.progress('deploying')
  const deployment = await api.deploy(appName, appImage)
  logger.success(`Successfully deployed version #${deployment.version}!`)
}

export async function deployAppDir(appDir) {
  const appYAML = loadAppYAML(appDir)
  let files = []

  for (const filepath of find(appDir)) {
    files.push({
      filepath,
      body: fs.readFileSync(path.join(appDir, filepath))
    })
  }

  await deploy(appYAML, files)
}
