<template>
<dashboard-layout title="Stores">
  <table class="stores">
    <thead>
      <td class="key">Key</td>
      <td class="type">Type</td>
      <td class="value">Value</td>
      <td class="buttons"></td>
    </thead>

    <tbody>
      <tr v-for="store in stores">
        <td>{{ store.key }}</td>
        <td>
          <select v-if="store.editing" v-model="store.data_type">
            <option value="string">String</option>
            <option value="integer">Integer</option>
            <option value="float">Float</option>
            <option value="bool">Boolean</option>
          </select>
          <span v-else>{{ store.data_type }}</span>
        </td>

        <td>
          <input v-if="store.editing" type="text" v-model="store.value" placeholder="Value (e.g. 123, true, hello world!)">
          <span v-else>{{ store.value }}</span>
        </td>

        <td class="buttons">
          <button v-if="store.editing" @click="updateStore(store)">save</button>
          <button v-else @click="store.editing = !store.editing">edit</button>
          <button v-if="store.editing" @click="store.editing = !store.editing">cancel</button>
          <button v-else @click="deleteStore(store)">delete</button>
        </td>
      </tr>
    </tbody>
  </table>

  <table class="new-store">
    <thead>
      <td class="key">Key</td>
      <td class="type">Type</td>
      <td class="value">Value</td>
      <td class="buttons"></td>
    </thead>

    <tbody>
      <tr>
        <td>
          <input type="text" v-model="newStore.key" placeholder="Value (e.g. message)">
        </td>

        <td>
          <select v-model="newStore.dataType">
            <option value="string">String</option>
            <option value="integer">Integer</option>
            <option value="float">Float</option>
            <option value="bool">Boolean</option>
          </select>
        </td>

        <td>
          <input type="text" v-model="newStore.value" placeholder="Value (e.g. 123, true, hello world!)">
        </td>

        <td class="buttons">
          <button @click="createStore(newStore)">create</button>
        </td>
      </tr>
    </tbody>
  </table>
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
        value: ""
      }
    };
  },
  methods: {
    createStore() {
      api.createAppStore(this.appName, this.newStore.key, this.newStore.dataType,
          this.newStore.value)
        .then(() => {
          this.refreshAppStores()
        })
        .catch(error => notify("error", error));


    },
    updateStore(store) {
      api.updateAppStore(this.appName, newStore.key, store.data_type, store.value)
        .then(() => {
          this.refreshAppStores()
        })
        .catch(error => notify("error", error));
    },
    deleteStore(store) {
      api.deleteAppStore(this.appName, store.key)
        .then(() => {
          this.refreshAppStores()
        })
        .catch(error => notify("error", error));
    },
    refreshAppStores() {
      api.getAppStores(this.appName)
        .then(r => {
          this.stores = r.json.map(store => {
            store.editing = false
            return store
          })
        })
        .catch(error => notify("error", error));
    }
  },
  beforeMount() {
    this.refreshAppStores()
  }
};
</script>

<style lang="scss" scoped>
@mixin store-table {
  width: 100%;
  border-collapse: collapse;


  thead {
    td {
      text-align: left;
      padding-bottom: 2px;
      margin-bottom: 5px;
    }

    .key {
      width: 25%;
    }

    .type {
      width: 20%;
    }

    .value {
      width: 35%;
    }

    .button {
      width: 20%;
    }
  }

  .buttons {
    text-align: right;
  }

  tbody {
    tr:last-child {
      border-spacing: 10px;
      padding-top: 10px;
    }
  }
}

.stores {
  @include store-table;

  thead {
    td {
      border-bottom: 1px solid #777;
    }
  }
}

.new-store {
  @include store-table;
  margin-top: 50px;

  thead {
    td {
      font-weight: 600;
    }
  }
}
</style>
