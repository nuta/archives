const webpack = require('webpack')
const path = require('path')

module.exports = {
  mode: 'spa',
  head: {
    title: 'MakeStack',
    meta: [
      { charset: 'utf-8' },
      { hid: 'description', name: 'description', content: 'A "batteries-included" IoT PaaS for super-rapid prototyping.' }
    ],
    link: [
      {
        rel: 'icon',
        type: 'image/png',
        href: '/favicon.png'
      },
      {
        rel: 'stylesheet',
        href: 'https://fonts.googleapis.com/css?family=Open+Sans:400,600|Roboto:600,900|Lato:300i|Source+Code+Pro:400,600'
      },
      {
        rel: 'stylesheet',
        href: 'https://use.fontawesome.com/releases/v5.0.6/css/all.css'
      }
    ],
    script: [
      { src: 'https://www.google.com/recaptcha/api.js' }
    ]
  },
  loading: { color: '#3B80f0' },
  modules: ['@nuxtjs/proxy'],
  build: {
    vendor: ['whatwg-fetch', 'jszip'],
    plugins: [
      new webpack.DefinePlugin({
        'PLATFORM': `'${process.env.PLATFORM}'`,
        'RECAPTCHA_SITEKEY': `'${process.env.RECAPTCHA_SITEKEY}'`,
        'DEFAULT_SERVER_URL': "'https://makestack.cloud'"
      })
    ],
    extend(config) {
      config.module.rules.push({
        test: /\.d\.ts$/,
        loader: 'raw-loader'
      })

      config.devServer = {
        disableHostCheck: true
      }

      if (process.env.PLATFORM === 'desktop') {
        config.target = 'electron-renderer'
        config.resolve.alias['platform$'] = path.resolve(__dirname, 'desktop-platform.js')
      } else {
        config.resolve.alias['platform$'] = path.resolve(__dirname, 'web-platform.js')
        config.resolve.alias['electron$'] = path.resolve(__dirname, 'web-electron-stub.js')
      }
    }
  },
  proxy: {
    '/api': 'http://localhost:3000'
  }
}
