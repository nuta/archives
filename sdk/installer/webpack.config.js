process.env.BABEL_ENV = 'renderer'
const production = (process.env.NODE_ENV === 'production')

const path = require('path')
const webpack = require('webpack')
const CopyWebpackPlugin = require('copy-webpack-plugin')
const HtmlWebpackPlugin = require('html-webpack-plugin')

module.exports = {
  target: 'electron-renderer',
  devtool: production ? undefined : '#cheap-module-eval-source-map',
  entry: {
    renderer: path.join(__dirname, 'renderer/index.js')
  },
  output: {
    filename: '[name].js',
    path: path.join(__dirname, 'dist')
  },
  resolve: {
    alias: {
      'renderer': path.join(__dirname, 'renderer'),
      'vue$': 'vue/dist/vue.esm.js',
      'views': path.join(__dirname, 'views'),
      'layouts': path.join(__dirname, 'layouts')
    },
    extensions: ['.js', '.vue']
  },
  module: {
    rules: [
      {
        test: /\.vue$/,
        use: {
          loader: 'vue-loader',
          options: {
            loaders: {
              js: ['babel-loader'],
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
          'style-loader',
          'css-loader',
          'postcss-loader',
          {
            loader: 'sass-loader',
            options: {
              includePaths: [path.resolve('css')]
            }
          }
        ]
      },
      {
        test: /\.js$/,
        use: 'babel-loader',
        exclude: /node_modules/
      }
    ]
  },
  plugins: [
    new HtmlWebpackPlugin({
      filename: 'index.html',
      template: path.resolve(__dirname, 'renderer/index.html')
    }),
    new CopyWebpackPlugin([{
      from: path.resolve(__dirname, 'main'),
      to: 'dest/main'
    }]),
    new webpack.DefinePlugin({
      'process.env.NODE_ENV': process.env.production
    }),
    new webpack.NoEmitOnErrorsPlugin()
  ]
}
