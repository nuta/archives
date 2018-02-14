<template>
  <dashboard-layout title="Devices" :appName="appName" inverted-bg="true">
    <header>
      <div class="title-header-container">
        <div class="title-header">
          <h2>{{ deviceName }}</h2>
          <div class="status">
            <i class="fas fa-circle" :class="{ online }"></i>
            {{ statusMsg }}
          </div>
        </div>
        <nuxt-link :to="{ name: 'apps-appName-devices', params: { name: appName } }">
          <i class="fas fa-chevron-left"></i>
          Return to Devices
        </nuxt-link>
      </div>
    </header>

    <tabs>
      <tab name="Overview">
        <div class="form-sections">
          <section>
            <header>
              <h1>Device ID</h1>
            </header>
            <main>
              <clipboardable :text="device.device_id"></clipboardable>
            </main>
          </section>
          <section>
            <header>
              <h1>Device Secret</h1>
            </header>
            <main>
              <clipboardable :text="device.device_secret" hidden="true"></clipboardable>
            </main>
          </section>
        </div>
      </tab>
      <tab name="Config">
        <table>
          <thead>
            <tr>
              <th>Key</th>
              <th>Type</th>
              <th>Value</th>
              <th class="actions-column"></th>
            </tr>
          </thead>

          <tbody>
            <tr v-for="config in configs" :key="config.key">
              <td>{{ config.key }}</td>
              <td>
                <select v-if="config.editing" v-model="config.data_type" class="small">
                  <option value="string">String</option>
                  <option value="integer">Integer</option>
                  <option value="float">Float</option>
                  <option value="bool">Boolean</option>
                </select>
                <span v-else>{{ config.data_type }}</span>
              </td>

              <td>
                <input v-if="config.editing" type="text" v-model="config.value" class="small">
                <code v-else>{{ config.value }}</code>
              </td>

              <td>
                <button v-if="config.editing" @click="updateConfig(config)" class="small">
                  <i class="fa fa-check" aria-hidden="true"></i>
                  Save
                </button>

                <button v-else @click="config.editing = !config.editing" class="small">
                  <i class="fa fa-pencil" aria-hidden="true"></i>
                  Edit
                </button>

                <button v-if="config.editing" @click="config.editing = !config.editing" class="small">
                  <i class="fa fa-ban" aria-hidden="true"></i>
                  Cancel
                </button>

                <button v-else @click="deleteConfig(config)" class="danger small">
                  <i class="fa fa-trash" aria-hidden="true"></i>
                </button>
              </td>
            </tr>
            <tr class="secondary-row">
                <td>
                  <input type="text" v-model="newConfig.key" placeholder="Key" class="inverted small">
                </td>
                <td>
                  <select v-model="newConfig.dataType" class="inverted small">
                    <option value="string">String</option>
                    <option value="integer">Integer</option>
                    <option value="float">Float</option>
                    <option value="bool">Boolean</option>
                  </select>
                </td>
                <td>
                  <input type="text" v-model="newConfig.value" placeholder="Value" class="inverted small">
                </td>
                <td>
                  <button @click="createConfig" class="primary small">
                    <i class="fas fa-plus"></i>
                    Add
                  </button>
                </td>
            </tr>
          </tbody>
        </table>
      </tab>
      <tab name="Settings">
        <div class="form-sections">
          <section>
            <div class="header">
              <h1>Remove device</h1>
              <p>This action can't be reverted. Be careful!</p>
            </div>
            <div class="content">
              <button class="danger simple" @click="removeDevice">Remove Device</button>
            </div>
          </section>
        </div>
      </tab>
    </tabs>
  </dashboard-layout>
</template>

<script>
import api from "~/assets/js/api"
import DashboardLayout from "~/components/dashboard-layout"
import Card from "~/components/card"
import Tabs from "~/components/tabs"
import Tab from "~/components/fragments/tab"
import Clipboardable from "~/components/clipboardable"

export default {
  components: { DashboardLayout, Card, Tabs, Tab, Clipboardable },
  data() {
    return {
      appName: this.$route.params.appName,
      deviceName: this.$route.params.deviceName,
      device: {},
      configs: [],
      newConfig: {
        dataType: 'string'
      }
   }
  },
  computed: {
    breadcrumb() {
      return [
        {
          title: 'Devices',
          to: {
            name: 'apps-appName-devices',
            params: {
              name: this.appName
            }
          }
        },
        {
          title: this.deviceName,
          to: {
            name: 'apps-appName-devices-deviceName',
            params: {
              name: this.appName,
              deviceName: this.deviceName
            }
          }
        }
      ]
    },
    online() {
      return this.device && ['running', 'booting'].includes(this.device.status)
    },
    statusMsg() {
      return 'unknown' // TODO
    }
  },
  methods: {
    async removeDevice() {
      await api.deleteDevice(this.deviceName)
    },
    async createConfig() {
      await api.updateDeviceConfig(
              this.deviceName,
              this.newConfig.key,
              this.newConfig.dataType,
              this.newConfig.value)

      await this.refreshDeviceConfigs()
    },
    async updateConfig(config) {
      await api.updateDeviceConfig(
              this.deviceName,
              config.key,
              config.data_type,
              config.value)
      await this.refreshDeviceConfigs()
    },
    async deleteConfig(config) {
      await api.deleteDeviceConfig(this.deviceName, config.key)
      await this.refreshDeviceConfigs()
    },
    async refreshDeviceConfigs() {
      this.configs = (await api.getDeviceConfigs(this.deviceName)).map(config => {
        config.editing = false
        return config
      })
    },
  },
  async beforeMount() {
    this.device = await api.getDevice(this.deviceName)
    await this.refreshDeviceConfigs()
  }
}
</script>

<style lang="scss" scoped>
.actions-column {
  width: 200px;
}

table {
  td:nth-child(4) {
    text-align: right;
  }
}

</style>
