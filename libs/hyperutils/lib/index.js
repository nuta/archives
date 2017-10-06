module.exports = {
  fetch: require('node-fetch'),
  ioctl: require('bindings')('ioctl.node').ioctl,
  NodeVM: require('vm2').NodeVM
}