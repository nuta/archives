let api = require("../api");

module.exports = (url, username, password) => {
  return api.login(url, username, password)
};