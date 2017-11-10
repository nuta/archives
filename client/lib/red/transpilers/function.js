module.exports = config => {
  return {
    type: 'function',
    code: `

 __outputs__([
  ((msg) => {
    ${config.code};
    return {payload: null}; // for safety
  })(__input__)
])
    `
  }
}
