module.exports = (RED) => {
  function HDC1000TemperatureNode(config) {
    RED.nodes.createNode(this, config)
  }

  RED.nodes.registerType('hdc1000-temperature', HDC1000TemperatureNode)
}
