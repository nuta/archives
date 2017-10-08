process.env.BABEL_ENV = 'main'
const production = process.env.NODE_ENV === 'production'

const path = require('path')
const { dependencies } = require('../package.json')
const webpack = require('webpack')
const BabiliWebpackPlugin = require('babili-webpack-plugin')

let mainConfig = {
  target: 'electron-main',
  entry: {
    main: path.join(__dirname, '../main/index.js')
  },
  module: {
    rules: [
      {
        test: /\.js$/,
        use: 'babel-loader',
        exclude: /node_modules/
      },
      {
        test: /\.node$/,
        use: 'node-loader'
      }
    ]
  },
  node: {
    __dirname:  !production,
    __filename: !production
  },
  output: {
    filename: '[name].js',
    libraryTarget: 'commonjs2',
    path: path.join(__dirname, '../dist/electron')
  },
  plugins: [
    new webpack.NoEmitOnErrorsPlugin()
  ],
  resolve: {
    extensions: ['.js', '.json', '.node']
  }
}

switch (process.env.NODE_ENV) {
  case 'production':
    mainConfig.plugins.push(
      new BabiliWebpackPlugin(),
      new webpack.DefinePlugin({
        'process.env.NODE_ENV': '"production"'
      })
    )
    break

  case 'development':
    mainConfig.plugins.push(
      new webpack.DefinePlugin({
        '__static': `"${path.join(__dirname, '../static').replace(/\\/g, '\\\\')}"`
      })
    )
    break
}

module.exports = mainConfig