/* Be careful! This script is executed in root!!! */
const { imageWriter } = require('makestack')

imageWriter().catch(error => {
  console.error(error)
  process.exit(1)
})
