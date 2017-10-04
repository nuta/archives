<template>
<dashboard-layout title="Stores">
  <div class="list">
    <form v-for="store in stores" v-on:submit.prevent="update(store)">
      <label>Key</label>
      <input type="text" v-model="store.key" placeholder="key (e.g. led-state)">

      <label>Value</label>
      <input type="text" v-model="store.value" placeholder="Value (e.g. 123, true, hello world!)">

      <label>Comment</label>
      <input type="text" v-model="store.comment" placeholder="Comment">

      <input type="submit" value="Update">
    </form>

    <form v-on:submit.prevent="create">
      <label>Key</label>
      <input type="text" v-model="newStore.key" placeholder="key (e.g. led-state)">

      <label>Type</label>
      <select v-model="newStore.dataType">
        <option value="string">String</option>
        <option value="integer">Integer</option>
        <option value="float">Float</option>
        <option value="bool">Boolean</option>
      </select>

      <label>Value</label>
      <input type="text" v-model="newStore.value" placeholder="Value (e.g. 123, true, hello world!)">

      <label>Comment</label>
      <input type="text" v-model="newStore.comment" placeholder="Comment">

      <input type="submit" value="Create a new store">
    </form>
  </div>
</dashboard-layout>
</template>

<script>
import api from "js/api";
import DashboardLayout from "layouts/dashboard";

export default {
  components: { DashboardLayout },
  data() {
    return {
      appName: app.$router.currentRoute.params.appName,
      stores: [],
      newStore: {
        key: "",
        dataType: "string",
        comment: ""
      }
    };
  },
  methods: {
    create() {
      api.createAppStore(this.appName, this.newStore.key, this.newStore.dataType,
          this.new_store.value)
        .catch(error => notify("error", error));


    },
    update(store) {
      api.updateAppStore(this.appName, store.key, store.dataType, store.value)
        .catch(error => notify("error", error));
    }
  },
  beforeMount() {
    api.getAppStores(this.appName)
      .then(r => this.stores = r.json)
      .catch(error => notify("error", error));
  }
};
</script>

<style lang="scss" scoped>
</style>
