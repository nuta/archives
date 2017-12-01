import * as fs from 'fs';
import * as path from 'path';
import { spawnSync } from 'child_process';
import * as JSZip from 'jszip';
import * as yaml from 'js-yaml';
import { api } from '../api';
import { createFile } from '../helpers';
import { logger } from '../logger';

function installDevDependencies() {
  const { status, stdout, stderr } = spawnSync('yarn', [], { encoding: 'utf-8' })
  if (status !== 0) {
    throw new Error(`yarn exited with ${status}:\n${stdout}\n${stderr}`)
  }
}

async function downloadAndExtractPlugin(plugin: string) {
  const pluginZip = await (new JSZip()).loadAsync(await api.downloadPlugin(plugin))

  for (const filepath in pluginZip.files) {
    if (!filepath.endsWith('/')) {
      console.log(filepath)
      const body = pluginZip.files[filepath].async('arraybuffer')
      createFile(path.join('node_modules/@makestack', plugin, filepath), body)
    }
  }
}

export async function main(args, opts) {
  const makestackTypeFile = path.resolve(__dirname, '../../runtime/makestack.d.ts')

  let yamlPath
  if (fs.existsSync('app.yaml')) {
    yamlPath = 'app.yaml'
  } else if (fs.existsSync('plugin.yaml')) {
    yamlPath = 'plugin.yaml'
  } else {
    throw new Error('The current directory is not an app nor a plugin.')
  }

  if (fs.existsSync('package.json')) {
    logger.info('Installing npm dev dependecies')
    installDevDependencies()
  }

  const { plugins } = yaml.safeLoad(fs.readFileSync(yamlPath))
  for (const plugin of plugins || []) {
    if (fs.existsSync(path.join('node_modules/@makestack', plugin))) {
      logger.progress(`Download @makestack/${plugin}`)
      downloadAndExtractPlugin(plugin)
    } else {
      logger.progress(`Download @makestack/${plugin} (alredy exists)`)
    }
  }

  logger.progress('Add @types/makestack')
  createFile('node_modules/@types/makestack/index.d.ts',
      fs.readFileSync(makestackTypeFile))
}
