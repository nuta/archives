module.exports = (RED) => {
  function GPIOOutNode(config) {
    RED.nodes.createNode(this, config)
  }

  RED.nodes.registerType('gpio-out', GPIOOutNode)
}
