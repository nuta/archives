const path = require('path')
const HtmlWebpackPlugin = require('html-webpack-plugin')

module.exports = {
  entry: {
    app: './ui/main.js',
    vendor: ['vue', 'vue-router']
  },
  output: {
    path: path.resolve('public'),
    publicPath: '/',
    filename: '[name].[chunkhash].js'
  },
  resolve: {
    extensions: ['.js', '.vue'],
    alias: {
      js: path.resolve('ui/js'),
      components: path.resolve('ui/components'),
      layouts: path.resolve('ui/layouts'),
      vue: 'vue/dist/vue.js'
    }
  },
  module: {
    loaders: [
      {
        test: /.vue$/,
        loader: 'vue-loader',
        options: {
          loaders: {
            js: ['babel-loader']
          }
        }
      },
      {
        test: /.html$/,
        loader: 'html-loader'
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
      }
    ]
  },
  plugins: [
    new HtmlWebpackPlugin({
      template: './ui/index.html',
      inject: true
    })
  ],
  devServer: {
    contentBase: 'public',
    host: '0.0.0.0',
    port: 8080,
    historyApiFallback: true,
    inline: true,
    proxy: {
      '/api/*': 'http://localhost:3000'
    }
  }
}
