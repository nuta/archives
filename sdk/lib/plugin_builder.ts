import * as fs from 'fs';
import * as os from 'os';
import * as path from 'path';
import { spawnSync } from 'child_process';

export function buildPlugin(pluginDir, destPath) {
  fs.copyFileSync(
    path.resolve(__dirname, '../runtime/makestack.d.ts'),
    path.resolve(__dirname, '../plugin_builder/makestack.d.ts')
  )

  spawnSync('docker', ['build', '-t', 'makestack/plugin-builder', '.'], {
    stdio: 'inherit',
    env: process.env, // DOCKER_HOST, etc.
    cwd: path.resolve(__dirname, '../plugin_builder')
  })

  // Note that docker-machine in macOS does not support mounting volumes
  // outside the home directory.
  const tempdir = path.join(os.homedir(), '.makestack', 'plugin-builder')

  spawnSync('docker', ['run', '--rm', '-v', `${path.resolve(pluginDir)}:/plugin:ro`,
    '-v', `${tempdir}:/dist`, '-t', 'makestack/plugin-builder'], {
    stdio: 'inherit',
    env: process.env // DOCKER_HOST, etc.
  })

  fs.copyFileSync(path.resolve(tempdir, 'plugin.zip'), destPath)
}
