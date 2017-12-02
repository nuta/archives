function generateCodeHeader() {
  return `\
const { AQM0802A } = require('@makestack/aqm0802a')
`
}

function generateCodeFooter() {
  return `\
const display = new AQM0802A()
Timer.interval(1, () => {
  display.update(Date.now().toString())
})
`
}

module.exports = { generateCodeHeader, generateCodeFooter }
