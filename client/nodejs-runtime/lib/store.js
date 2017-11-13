module.exports = class {
  constructor() {
    this.stores = {}
    this.onChangeCallbacks = {}
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

  update(newStores) {
    for (const key in newStores) {
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
