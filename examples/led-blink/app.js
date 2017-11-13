const led = new GPIO({ pin: 13, mode: GPIO.OUTPUT })

let state = false
Timer.interval(1, () => {
  led.write(state)
  state = !state
})
