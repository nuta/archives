const fs = require('fs')
const os = require('os')
const path = require('path')
const { spawnSync } = require('child_process')

module.exports = (pluginDir, destPath) => {
  spawnSync('docker', ['build', '-t', 'makestack/plugin-builder', '-'], {
    input: fs.readFileSync(path.resolve(__dirname, 'plugin_builder.Dockerfile')),
    stdio: ['pipe', 'inherit', 'inherit'],
    env: process.env // DOCKER_HOST, etc.
  })

  // Note that docker-machine in macOS does not mounting volumes outside home directory.
  const tempdir = path.join(os.homedir(), '.makestack', 'plugin-builder')

  spawnSync('docker', ['run', '--rm', '-v', `${path.resolve(pluginDir)}:/plugin:ro`,
    '-v', `${tempdir}:/dist`, '-t', 'makestack/plugin-builder'], {
    stdio: 'inherit',
    env: process.env // DOCKER_HOST, etc.
  })

  fs.copyFileSync(path.resolve(tempdir, 'plugin.zip'), destPath)
}
