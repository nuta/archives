module.exports = config => {
  let unit
  switch (config.unit) {
    case 's': unit = 1000; break;
    case 'm': unit = 1000 * 60; break;
    case 'h': unit = 1000 * 60 * 60; break;
    default: unit = 1000
  }

  const interval = (parseInt(config.interval) || 1) * unit

  return {
    type: 'input',
    code: `
setInterval(() => {
  __outputs__([{ topic: \`${config.topic}\`, payload: \`${config.payload}\` }])
}, ${interval})
`
  }
}
