module.exports = config => {
  return {
    type: 'output',
    modules: ['util'],
    init: '',
    oninput: `println(util.inspect(__input__.payload))`
  }
}
