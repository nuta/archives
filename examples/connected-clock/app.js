const AQM0802A = require('@makestack/aqm0802a')
const HDC1000 = require('@makestack/hdc1000')

const display = new AQM0802A()
const sensor = new HDC1000()

let displayMessages = []

Config.onChange('messages', newMessages => {
  displayMessages = JSON.parse(newMessages)
})

Timer.loop(async() => {
  if (displayMessages.length === 0) {
    await Timer.sleep(3)
    return
  }

  for (const text of displayMessages) {
    display.update(text)
    await Timer.sleep(3)
  }
})

Timer.interval(5, () => {
  publish('t', sensor.readTemperature())
  publish('h', sensor.readHumidity())
})
