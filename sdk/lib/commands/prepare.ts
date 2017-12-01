import * as fs from 'fs';
import * as path from 'path';
import { spawnSync } from 'child_process';
import { createFile } from '../helpers';
import { logger } from '../logger';

export function main(args, opts) {
  const makestackTypeFile = path.resolve(__dirname, '../../makestack.d.ts')

  if (!fs.existsSync('app.yaml') && !fs.existsSync('plugin.yaml')) {
    throw new Error('The current directory is not an app nor a plugin.')
  }

  if (fs.existsSync('package.json')) {
    logger.info('Installing npm dev dependecies')
    const { status, stdout, stderr } = spawnSync('yarn', [], { encoding: 'utf-8' })
    if (status !== 0) {
      throw new Error(`yarn exited with ${status}:\n${stdout}\n${stderr}`)
    }
  }

  logger.info('Add @types/makestack')
  createFile('node_modules/@types/makestack/index.d.ts',
      fs.readFileSync(makestackTypeFile))
}
