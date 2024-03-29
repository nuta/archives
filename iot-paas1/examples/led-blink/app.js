const { GPIO, Timer } = require('makestack')
const led = new GPIO({ pin: 13, mode: 'out' })

let state = false
Timer.interval(1, () => {
  led.write(state)
  state = !state
})
