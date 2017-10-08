process.env.BABEL_ENV = 'renderer'
const production = process.env.NODE_ENV === 'production'

const path = require('path')
const { dependencies } = require('../package.json')
const webpack = require('webpack')
const BabiliWebpackPlugin = require('babili-webpack-plugin')
const CopyWebpackPlugin = require('copy-webpack-plugin')
const HtmlWebpackPlugin = require('html-webpack-plugin')

let rendererConfig = {
  target: 'electron-renderer',
  entry: {
    renderer: path.join(__dirname, '../renderer/index.js')
  },
  module: {
    rules: [
      {
        test: /\.vue$/,
        use: {
          loader: 'vue-loader',
          options: {
            loaders: {
              js: ["babel-loader"],
              scss: 'vue-style-loader!css-loader!sass-loader'
            }
          }
        }
      },
      {
        test: /\.html$/,
        use: 'html-loader'
      },
      {
        test: /\.scss$/,
        use: [
            "style-loader",
            "css-loader",
            "postcss-loader",
            {
                loader: "sass-loader",
                options: {
                    includePaths: [path.resolve("css")]
                }
            }
        ]
      },
      {
        test: /\.js$/,
        use: 'babel-loader',
        exclude: /node_modules/
      },
      {
        test: /\.node$/,
        use: 'node-loader'
      },
      {
        test: /\.(svg)(\?.*)?$/,
        use: {
          loader: 'url-loader',
          query: {
            limit: 10000,
            name: 'assets/[name]--[folder].[ext]'
          }
        }
      }
    ]
  },
  node: {
    __dirname:  !production,
    __filename: !production
  },
  plugins: [
    new HtmlWebpackPlugin({
      filename: 'index.html',
      template: path.resolve(__dirname, '../renderer/index.html'),
      nodeModules: production ? false : path.resolve(__dirname, '../node_modules')
    }),
    new webpack.HotModuleReplacementPlugin(),
    new webpack.NoEmitOnErrorsPlugin()
  ],
  output: {
    filename: '[name].js',
    libraryTarget: 'commonjs2',
    path: path.join(__dirname, '../dist/electron')
  },
  resolve: {
    alias: {
      '@': path.join(__dirname, '../src/renderer'),
      'vue$': 'vue/dist/vue.esm.js',
      'views': path.join(__dirname, '../views')
    },
    extensions: ['.js', '.vue', '.json', '.css', '.node']
  },
}

switch(process.env.NODE_ENV) {
  case 'production': 
    rendererConfig.plugins.push(
      new BabiliWebpackPlugin(),
      new CopyWebpackPlugin([
        {
          from: path.join(__dirname, '../assets'),
          to: path.join(__dirname, '../dist/electron/assets'),
          ignore: ['.*']
        }
      ]),
      new webpack.DefinePlugin({
        'process.env.NODE_ENV': '"production"'
      }),
      new webpack.LoaderOptionsPlugin({
        minimize: true
      })
    )
    break

  case 'development':
    rendererConfig.devtool = '#cheap-module-eval-source-map'
    rendererConfig.plugins.push(
      new webpack.DefinePlugin({
        '__assets': `"${path.join(__dirname, '../assets').replace(/\\/g, '\\\\')}"`
      })
    )
    break
}

module.exports = rendererConfig