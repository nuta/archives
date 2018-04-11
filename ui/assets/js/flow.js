export const defs = {
  inject: {
    category: 'input',
    inputs: 1,
    outputs: 0,
    icon: 'fa-angle-double-right',
    name: 'inject',
    options: {
      type: {
        name: 'type',
        description: 'The value type.',
        type: 'select',
        options: ['string', 'number'],
        required: true
      },
      value: {
        name: 'value',
        description: 'The value to inject.',
        type: 'text',
        required: true
      },
      interval: {
        name: 'interval',
        description: 'The interval in seconds.',
        type: 'integer',
        required: true
      }
    },
    transpile(id, node) {
      const value = node.options.value
      const interval = parseInt(node.options.interval) || 1
      return {
        onload: `\
setInterval(() => {
  __output__('${id}', [${JSON.stringify(value)}])
}, ${interval * 1000})
`,
        oninput: ''
      }
    }
  },
  log: {
    category: 'output',
    inputs: 1,
    outputs: 0,
    icon: 'fa-angle-double-right',
    name: 'log',
    options: {
    },
    transpile(id, node) {
      return {
        onload: '',
        oninput: 'print(input)'
      }
    }
  }
}

export function buildFlowApp(nodes) {
  /* Code must be written in ES5 to support JerryScript (ESP-32). */
  let code = `\
const EventEmitter = require('events')

const __nodes__ = ${JSON.stringify(nodes)};

function __output__(id, outputs) {
  for (var i = 0; i < outputs.length; i++) {
    var dests = __nodes__[id].outputs[i];
    for (var j = 0; j < dests.length; j++) {
      __nodes__[dests[i].node].ev.emit('input', outputs[i], dests[i].port)
    }
  }
}

`

  for (const [id, node] of Object.entries(nodes)) {
    const { onload, oninput } = defs[node.type].transpile(id, node)

    code += `
const ev_${id} = new EventEmitter()
__nodes__['${id}'] = {
    ev: ev_${id},
    type: '${node.type}'
}

${onload}
ev_${id}.on('input', input => {
    ${oninput}
})
`
  }

  return code
}
