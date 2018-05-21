function generateCodeHeader() {
  return `\
const { HDC1000 } = require('@makestack/hdc1000')
`
}

function generateCodeFooter() {
  return `\
const hdc1000 = new HDC1000()
Timer.interval(5, () => {
  publish('temperature', hdc1000.readTemperature())
})
`
}

module.exports = { generateCodeHeader, generateCodeFooter }
