module.exports = (RED) => {
  function HDC1000HumidityNode(config) {
    RED.nodes.createNode(this, config)
  }

  RED.nodes.registerType('hdc1000-humidity', HDC1000HumidityNode)
}
