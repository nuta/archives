module.exports = config => {
  const instance = `__hdc1000_humid_${config.nodeId}__`
  return {
    type: 'function',
    plugins: ['hdc1000'],
    init: `const ${instance} = new hdc1000()`,
    oninput: `__outputs__([{ payload: ${instance}.read_humidity() }])`
  }
}
