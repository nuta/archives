module.exports = class {
  constructor() {
    this.stores = {}
  }

  onchange(key, callback) {
    if (key in this.stores) {
      this.stores[key].onchangeCallbacks = callback
    } else {
      this.stores[key] = {
        value: null,
        onchangeCallbacks: []
      }
    }
  }

  update(newStores) {
    for (let key in newStores) {
      let newValue = newStores[key]
      let store = this.stores[key]
      if (!store) { continue }

      let oldValue = store.value
      store.value = newValue

      if (oldValue != null && oldValue !== newValue) {
        let callbacks = store.onchangeCallbacks
        for (let i = 0; i < callbacks.length; i++) {
          callbacks[i](newValue)
        }
      }
    }
  }
}
