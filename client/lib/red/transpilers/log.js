module.exports = config => {
  return {
    type: 'output',
    init: '',
    oninput: `
if (__input__ && typeof __input__.payload === 'string') {
  Logging.print(__input__.payload)
} else {
  console.warn('log:' , 'invalid payload:', __input__)
}
`
  }
}
