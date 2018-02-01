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
        href: 'https://fonts.googleapis.com/css?family=Open+Sans:400,600|Roboto:900|Lato:300i'
      },
      {
        rel: 'stylesheet',
        href: 'https://cdnjs.cloudflare.com/ajax/libs/normalize/7.0.0/normalize.min.css',
        integrity: 'sha256-HxaKz5E/eBbvhGMNwhWRPrAR9i/lG1JeT4mD6hCQ7s4',
        crossorigin: 'anonymous'
      },
      {
        rel: 'stylesheet',
        href: 'https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css',
        integrity: 'sha256-eZrrJcwDc/3uDhsdt61sL2oOBY362qM3lon1gyExkL0: ',
        crossorigin: 'anonymous'
      },
      {
        rel: 'stylesheet',
        href: 'https://maxcdn.bootstrapcdn.com/bootstrap/4.0.0/css/bootstrap.min.css',
        integrity: 'sha384-Gn5384xqQ1aoWXA+058RXPxPg6fy4IWvTNh0E263XmFcJlSAwiGgFAW/dAiS6JXm',
        crossorigin: 'anonymous'
      }
    ],
    script: [
      {
        src: 'https://cdnjs.cloudflare.com/ajax/libs/require.js/2.3.5/require.min.js',
        integrity: 'sha256-0SGl1PJNDyJwcV5T+weg2zpEMrh7xvlwO4oXgvZCeZk=',
        crossorigin: 'anonymous'
      }
    ]
  },
  loading: { color: '#3B80f0' },
  modules: [ '@nuxtjs/proxy' ],
  build: {
    vendor: ['whatwg-fetch', 'jszip', 'date-fns', 'chart.js', 'lodash'],
    plugins: [
      new CopyWebpackPlugin([
        {
          from: 'node_modules/monaco-editor/min/vs',
          to: 'monaco-editor/vs'
        }
      ])
    ],
    extend(config) {
      config.module.rules.push({
        test: /\.d\.ts$/,
        loader: 'raw-loader',
        exclude: /node_modules/
      })

      config.resolve.alias['vs'] = path.resolve(__dirname, 'node_modules/monaco-editor/dev/vs')

      config.devServer = {
        disableHostCheck: true
      }

      if (process.env.PLATFORM === 'desktop') {
        config.target = 'electron-renderer'
        config.resolve.alias['platform$'] = path.resolve(__dirname, 'desktop-platform.js')
      } else {
        config.resolve.alias['platform$'] = path.resolve(__dirname, 'browser-platform.js')
      }
    }
  },
  proxy: {
    '/api': 'http://localhost:3000'
  }
}
