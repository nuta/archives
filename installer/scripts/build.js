const fs = require('fs')
const path = require('path')
const { execFileSync } = require('child_process')

execFileSync(path.join(__dirname, '../node_modules/.bin/webpack'),
  ['--config', 'webpack/webpack.main.config.js'],
  { stdio: 'inherit' })
  
execFileSync(path.join(__dirname, '../node_modules/.bin/webpack'),
  ['--config', 'webpack/webpack.renderer.config.js'],
  { stdio: 'inherit' })
    
execFileSync(path.join(__dirname, '../node_modules/.bin/electron-packager'),
  ['.', '--out', 'dist', '--arch', 'x64',
   '--platform', process.env.PLATFORM, '--asar', '--overwrite'],
  { stdio: 'inherit' })
    