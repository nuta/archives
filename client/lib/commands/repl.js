const readline = require('readline')
const util = require('util')
const api = require('../api')

async function input(rl) {
  return new Promise((resolve, reject) => {
    rl.question('>>> ', resolve)
  })
}

async function sleep(sec) {
  return new Promise((resolve, reject) => setTimeout(resolve, sec * 1000))
}

module.exports = async (args, opts, logger) => {
  const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout
  })

  while (true) {
    const line = await input(rl)
    const { id } = await api.invokeCommand(args.name, '__repl__', line)
    while (true) {
      const results = (await api.getCommandResults(args.name)).results
      if (id in results) {
        const result = JSON.parse(results[id].split('__repl__ ')[1])
        console.log(util.inspect(result, { colors: true }))
        break
      }
      await sleep(1)
    }
  }
}
