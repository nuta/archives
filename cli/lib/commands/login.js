const login = require('../login')
const inquirer = require('inquirer')

module.exports = async (args, opts, logger) => {
  const answers = await inquirer.prompt([
    { message: "Server URL", name: "url", type: "input" },
    { message: "Username", name: "username", type: "input" },
    { message: "Password", name: "password", type: "password" }
  ])
  
  try {
    await login(answers.url, answers.username, answers.password)
  } catch (e) {
    logger.error('failed to login', e)
    process.exit(1)
  }
  
  logger.info("Welcome to MakeStack!");
}
