module.exports = {
  fetch: require('node-fetch'),
  msgpack: require('msgpack'),
  ioctl: require('bindings')('ioctl.node').ioctl
}