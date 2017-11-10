module.exports = config => {
  if (!parseInt(config.pin)) {
    throw new Error('gpio-in: pin is not set')
  }

  const instance = `__gpio_input_${config.nodeId}__`
  return {
    type: 'input',
    init: `
const ${instance} = new GPIO({ pin: ${config.pin}, mode: GPIO.INPUT })
${instance}.onChange(() => {
  const value = ${instance}.read()
  __outputs__([{ payload: value ? 1 : 0 }])
})
`
  }
}
