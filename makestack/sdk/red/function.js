module.exports = config => {
  return {
    type: 'function',
    init: '',
    oninput: `
 __outputs__([
  ((msg) => {
    ${config.code};
    return {payload: null}; // for safety
  })(__input__)
])
    `
  }
}
