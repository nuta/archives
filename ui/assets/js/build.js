const JSZip = require('jszip')

export async function buildApp(code) {
  let zip = new JSZip()

  // Copy app files.
  zip.file('app.js', code)

  const zipImage = await zip.generateAsync({
    type: 'arraybuffer',
    compression: 'DEFLATE',
    compressionOptions: {
      level: 9
    }
  })

  return new Blob([zipImage], { type: 'application/zip' })
}
