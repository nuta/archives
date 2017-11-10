module.exports = (RED) => {
  function FunctionNode(config) {
    RED.nodes.createNode(this, config)
  }

  RED.nodes.registerType('function', FunctionNode)
}
