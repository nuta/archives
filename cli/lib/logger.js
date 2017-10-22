const { createLogger, transports, addColors, format } = require('winston')

const levels = {
  error: 1,
  warn: 2,
  progress: 3,
  info: 4,
  recommend: 5,
  debug: 6
}

const colors = {
  error: 'red',
  warn: 'yellow',
  progress: 'blue',
  recommend: 'yellow',
  debug: 'magenta'
}

addColors(colors)

module.exports = createLogger({
  level: process.env.LOG_LEVEL || 'info',
  levels: levels,
  transports: [
    new transports.Console()
  ],
  format: format.combine(
    format.colorize(),
    format.printf(info => `[${info.level.padEnd(20)}] ${info.message}`)
  )
})
