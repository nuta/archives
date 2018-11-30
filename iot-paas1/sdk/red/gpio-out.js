module.exports = config => {
  const instance = `__gpio_input_${config.nodeId}__`
  return {
    type: 'input',
    init: `const ${instance} = new GPIO({ pin: ${config.pin}, mode: 'out' })`,
    oninput: `console.log(__input__); ${instance}.write(__input__.payload == '1')`
  }
}
