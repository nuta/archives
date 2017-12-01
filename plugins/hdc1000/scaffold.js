const header = `\
const { HDC1000 } = require('@makestack/hdc1000')
`

const footer = `\
const hdc1000 = new HDC1000()
Timer.interval(5, () => {
  publish('temperature', hdc1000.readTemperature())
})
`

function scaffold(context) {
  return { header, footer }
}

module.exports = { scaffold }
