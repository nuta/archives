const util = require('util')


function print(level, ...messages) {
  const colors = {
    'DEBUG': '35',
    'INFO': '34',
    'ERROR': '31',
    'WARN': '33'
  }

  let body = ''
  for (const message of messages) {
    body += ((typeof message === 'string') ? message : util.inspect(message)) + ' '
  }

  const time = (new Date()).toTimeString().split(' ')[0]
  const color = colors[level] || ''
  console.log(`[\x1b[1;34msupervisor\x1b[0m ${time} \x1b[${color}m${level}\x1b[0m] ${body}`)
}

function debug(...messages) {
  print('DEBUG', ...messages)
}

function info(...messages) {
  print('INFO', ...messages)
}

function error(...messages) {
  print('ERROR', ...messages)
}

function warn(...messages) {
  print('WARN', ...messages)
}

module.exports = {
  debug,
  info,
  error,
  warn
}
