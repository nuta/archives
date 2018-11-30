/* Be careful! This script is executed in root!!! */
const { imageWriter } = require('makestack-sdk')

imageWriter().catch(error => {
  console.error(error)
  process.exit(1)
})
