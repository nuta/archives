module.exports = class {
  constructor() {
    this.stores = {}
    this.onChangeCallbacks = {}
    this.onCommandCallbacks = {}
  }

  onCommand(key, callback) {
    this.onCommandCallbacks[key] = callback
  }

  onChange(key, callback) {
    if (this.stores[key] !== undefined) {
      callback(this.stores[key])
    }

    if (key in this.onChangeCallbacks) {
      this.onChangeCallbacks[key].push(callback)
    } else {
      this.onChangeCallbacks[key] = [callback]
    }
  }

  async update(newStores) {
    for (const key in newStores) {
      if (key.startsWith('>')) {
        // Command
        const [commandId, commandKey] = key.substring(1).split(' ')
        if (this.onCommandCallbacks[commandKey]) {
          const returnValue = await this.onCommandCallbacks[commandKey](newStores[key])
          process.send({ type: 'log', body: `<${commandId} ${returnValue}` })
        }
      } else {
        // Store
        const oldValue = this.stores[key]
        const newValue = newStores[key]
        this.stores[key] = newValue

        if (this.onChangeCallbacks[key] && oldValue !== newValue) {
          for (const callback of this.onChangeCallbacks[key]) {
            callback(newValue)
          }
        }
      }
    }
  }
}
