module.exports = (RED) => {
  function GPIOInNode(config) {
    RED.nodes.createNode(this, config)
  }

  RED.nodes.registerType('gpio-in', GPIOInNode)
}
