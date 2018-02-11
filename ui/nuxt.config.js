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
        rel: 'stylesheet',
        href: 'https://fonts.googleapis.com/css?family=Open+Sans:400,600|Roboto:600,900|Lato:300i'
      },
      {
        rel: 'stylesheet',
        href: 'https://fonts.googleapis.com/css?family=Source+Code+Pro:400,600'
      },
      {
        rel: 'stylesheet',
        href: 'https://use.fontawesome.com/releases/v5.0.6/css/all.css'
      },
      {
        rel: 'stylesheet',
        href: 'https://cdnjs.cloudflare.com/ajax/libs/normalize/7.0.0/normalize.min.css',
        integrity: 'sha256-HxaKz5E/eBbvhGMNwhWRPrAR9i/lG1JeT4mD6hCQ7s4',
        crossorigin: 'anonymous'
      },
      {
        rel: 'stylesheet',
        href: 'https://cdnjs.cloudflare.com/ajax/libs/balloon-css/0.5.0/balloon.min.css'
      }
    ],
    script: [
      { src: 'https://www.google.com/recaptcha/api.js' },
      { src: 'https://cdnjs.cloudflare.com/ajax/libs/monaco-editor/0.10.1/min/vs/loader.js' }
    ]
  },
  loading: { color: '#3B80f0' },
  modules: ['@nuxtjs/proxy'],
  buildDir: process.env.BUILD_DIR || 'dist',
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
      config.devServer = {
        disableHostCheck: true
      }

      if (process.env.PLATFORM === 'desktop') {
        config.target = 'electron-renderer'
        config.resolve.alias['platform$'] = path.resolve(__dirname, 'desktop-platform.js')
      } else {
        config.resolve.alias['platform$'] = path.resolve(__dirname, 'browser-platform.js')
        config.resolve.alias['electron$'] = path.resolve(__dirname, 'web-electron-stub.js')
      }
    }
  },
  proxy: {
    '/api': 'http://localhost:3000'
  }
}
