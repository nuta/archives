let inquirer = require("inquirer");
let api = require("./api");

module.exports = (args, opts, logger) => {
    inquirer.prompt([
        { message: "Username", name: "username", type: "input" },
        { message: "Password", name: "password", type: "password" }
    ]).then(answers => {
        api.login(answers.username, answers.password).then(() => {
            logger.info("Welcome to MakeStack!");
        }).catch(e => {
            logger.error("failed to login", e);
        });
    });
};