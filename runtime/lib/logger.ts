import * as util from 'util';

function log(level: 'DEBUG' |  'INFO' | 'ERROR' | 'WARN', ...messages: string[]) {
  const colors = {
    'DEBUG': '35',
    'INFO': '34',
    'ERROR': '31',
    'WARN': '33'
  }

  let body = ''
  for (const message of messages) {
    let fragment
    if (typeof message === 'string') {
      fragment = message
    } else {
      fragment = util.inspect(message)
    }

    body += fragment + ' '
  }

  const time = (new Date()).toTimeString().split(' ')[0]
  const color = colors[level] || ''
  console.log(`[\x1b[1;32mruntime\x1b[0m ${time} \x1b[${color}m${level}\x1b[0m] ${body}`)
}

export function debug(...messages) {
  log('DEBUG', ...messages)
}

export function info(...messages) {
  log('INFO', ...messages)
}

export function error(...messages) {
  log('ERROR', ...messages)
}

export function warn(...messages) {
  log('WARN', ...messages)
}
