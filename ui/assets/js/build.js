const JSZip = require('jszip')

const startJs = `
function __handleError(error) {
  console.error(error.stack)
  process.send({
    type: 'log',
    body: '!' + error.stack.replace(/\\n/g, "\\n\\\\")
  })
  process.exit(1)
}

process.on('unhandledRejection', __handleError)
process.on('uncaughtException', __handleError)
require('./app')
`

export async function buildApp(code) {
  let zip = new JSZip()

  // Copy app files.
  zip.file('app.js', code)
  zip.file('start.js', startJs)

  const zipImage = await zip.generateAsync({
    type: 'arraybuffer',
    compression: 'DEFLATE',
    compressionOptions: {
      level: 9
    }
  })

  return new Blob([zipImage], { type: 'application/zip' })
}
