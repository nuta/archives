const fs = require('fs')
const path = require('path')
const { execFileSync } = require('child_process')

execFileSync(path.join(__dirname, '../node_modules/.bin/webpack'),
  ['--config', 'webpack/webpack.main.config.js'],
  { stdio: 'inherit' })
  
execFileSync(path.join(__dirname, '../node_modules/.bin/webpack'),
  ['--config', 'webpack/webpack.renderer.config.js'],
  { stdio: 'inherit' })
  
packageJson = JSON.parse(fs.readFileSync(path.join(__dirname, '../package.json')))
packageJson.main = 'main.js'
fs.writeFileSync('dist/electron/package.json', JSON.stringify(packageJson))
    
execFileSync(path.join(__dirname, '../node_modules/.bin/electron-packager'),
  ['dist/electron', '--out', 'dist', '--arch', 'x64',
   '--platform', process.env.PLATFORM, '--asar', '--overwrite'],
  { stdio: 'inherit' })
    