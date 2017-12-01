const header = `\
const { AQM0802A } = require('@makestack/aqm0802a')
`

const footer = `\
const display = new AQM0802A()
Timer.interval(1, () => {
  display.update(Date.now().toString())
})
`

function scaffold(context) {
  return { header, footer }
}

module.exports = { scaffold }
