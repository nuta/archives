// This is not a Node-RED node.
module.exports = (RED) => {
  process.on('message', ({ type, log }) => {
    if (type === 'log') {
      RED.comms.publish('log', log)
    }
  })
}
