module.exports = (RED) => {
  function InjectNode(config) {
    RED.nodes.createNode(this, config)
  }

  RED.nodes.registerType('inject', InjectNode)
}
