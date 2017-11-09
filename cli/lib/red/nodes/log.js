module.exports = (RED) => {
  function LogNode(config) {
    RED.nodes.createNode(this, config)
  }

  RED.nodes.registerType('log', LogNode)
}
