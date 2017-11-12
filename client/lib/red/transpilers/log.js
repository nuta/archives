module.exports = config => {
  return {
    type: 'output',
    modules: ['util'],
    init: '',
    oninput: `Logging.print(util.inspect(__input__.payload))`
  }
}
