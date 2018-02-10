<template>
  <dashboard-layout title="Settings" :appName="appName">
    <header>
      <h2>{{ appName }}</h2>
    </header>
    <nuxt-link :to="{ name: 'apps-appName', params: { name: appName } }">
      <i class="fas fa-chevron-left"></i>
      Return to Code
    </nuxt-link>

    <tabs>
      <tab name="Integrations">
        <div class="sections">
          <section>
            <header>
              <h1>IFTTT</h1>
              <span class="integration-status" :class="{ activated: ifttt.activated }">
                <i class="fas" :class="[ifttt.activated ? 'fa-check' : 'fa-times']"></i>
                {{ ifttt.activated ? 'activated' : 'unconfigured' }}
              </span>
            </header>
            <div class="content">
              <p>(<a href="https://platform.ifttt.com/maker">https://platform.ifttt.com/maker</a>)</p>
              <form @submit.prevent="updateIftttIntegration" v-if="!ifttt.activated">
                <input type="text" v-model="ifttt.key" placeholder="IFTTT Key">
                <input type="submit" value="Save" class="primary">
              </form>
              <div v-else>
                <p>IFTTT integration is activated. The API key is <code>{{ ifttt.key }}</code></p>
                <button @click="deleteIftttIntegration" class="danger">Delete IFTTT Integration</button>
              </div>
            </div>
          </section>
          <section>
            <header>
              <h1>ThingSpeak</h1>
              <span class="integration-status" :class="{ activated: thingSpeak.activated }">
                <i class="fas" :class="[thingSpeak.activated ? 'fa-check' : 'fa-times']"></i>
                {{ thingSpeak.activated ? 'activated' : 'unconfigured' }}
              </span>
            </header>
            <div class="content">
              <p>(<a href="https://platform.ifttt.com/maker">https://thingspeak.com</a>)</p>
              <form @submit.prevent="updateThingSpeakIntegration" v-if="!thingSpeak.activated">
                <input type="text" v-model="thingSpeak.write_api_key" placeholder="ThingSpeak Write API Key">
                <input type="submit" value="Save" class="primary">
              </form>
              <div v-else>
                <p>ThingSpeak integration is activated. The write API key is <code>{{ thingSpeak.write_api_key }}</code></p>
                <button @click="deleteThingSpeakIntegration" class="danger">Delete ThingSpeak Integration</button>
              </div>
            </div>
          </section>
          <section>
            <header>
              <h1>Outgoing Webhook</h1>
              <span class="integration-status" :class="{ activated: outgoingWebhook.activated }">
                <i class="fas" :class="[outgoingWebhook.activated ? 'fa-check' : 'fa-times']"></i>
                {{ outgoingWebhook.activated ? 'activated' : 'unconfigured' }}
              </span>
            </header>
            <div class="content">
              <p>A POST request will be sent.</p>
              <form @submit.prevent="updateOutgoingWebhookIntegration" v-if="!outgoingWebhook.activated">
                <input type="text" v-model="outgoingWebhook.url" placeholder="Webhook URL">
                <input type="submit" value="Save" class="primary">
              </form>
              <div v-else>
                <p>Outgoing Webhook integration is activated: The URL is <code>{{ outgoingWebhook.url }}</code></p>
                <button @click="deleteOutgoingWebhookIntegration" class="danger">Delete Outgoing Webhook Integration</button>
              </div>
            </div>
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
                <select v-if="config.editing" v-model="config.data_type">
                  <option value="string">String</option>
                  <option value="integer">Integer</option>
                  <option value="float">Float</option>
                  <option value="bool">Boolean</option>
                </select>
                <span v-else>{{ config.data_type }}</span>
              </td>

              <td>
                <input v-if="config.editing" type="text" v-model="config.value">
                <code v-else>{{ config.value }}</code>
              </td>

              <td class="actions">
                <button v-if="config.editing" @click="updateConfig(config)" class="btn btn-primary">
                  <i class="fa fa-check" aria-hidden="true"></i>
                  Save
                </button>

                <button v-else @click="config.editing = !config.editing" class="btn btn-default">
                  <i class="fa fa-pencil" aria-hidden="true"></i>
                  Edit
                </button>

                <button v-if="config.editing" @click="config.editing = !config.editing">
                  <i class="fa fa-ban" aria-hidden="true"></i>
                  Cancel
                </button>

                <button v-else @click="deleteConfig(config)" class="btn btn-danger">
                  <i class="fa fa-trash" aria-hidden="true"></i>
                </button>
              </td>
            </tr>
            <tr class="secondary-row">
                <td>
                  <input type="text" v-model="newConfig.key" placeholder="Key" class="small">
                </td>
                <td>
                  <select v-model="newConfig.dataType" class="small">
                    <option value="string">String</option>
                    <option value="integer">Integer</option>
                    <option value="float">Float</option>
                    <option value="bool">Boolean</option>
                  </select>
                </td>
                <td>
                  <input type="text" v-model="newConfig.value" placeholder="Value" class="small">
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
              <h3>Remove app</h3>
              <p>This action can't be reverted. Be careful!</p>
            </div>
            <div class="content">
              <button class="danger simple" @click="removeApp">Remove App</button>
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
import Tabs from "~/components/tabs"
import Tab from "~/components/fragments/tab"

export default {
  components: { DashboardLayout, Tabs, Tab },
  data() {
    return {
      appName: this.$route.params.appName,
      app: {},
      configs: [],
      newConfig: {
        type: 'string'
      },
      ifttt: {
        activated: false,
        key: ''
      },
      thingSpeak: {
        activated: false,
        write_api_key: ''
      },
      outgoingWebhook: {
        activated: false,
        url: ''
      }
    }
  },
  methods: {
    async removeApp() {
      await api.deleteApp(this.appName)
    },
    async createConfig() {
      await api.updateAppConfig(
              this.appName,
              this.newConfig.key,
              this.newConfig.dataType,
              this.newConfig.value)

      await this.refreshAppConfigs()
    },
    async updateConfig(config) {
      await api.updateAppConfig(
              this.appName,
              config.key,
              config.data_type,
              config.value)
      await this.refreshAppConfigs()
    },
    async deleteConfig(config) {
      await api.deleteAppConfig(this.appName, config.key)
      await this.refreshAppConfigs()
    },
    async refreshAppConfigs() {
      this.configs = (await api.getAppConfigs(this.appName)).map(config => {
        config.editing = false
        return config
      })
    },
    async createIftttIntegration() {
      await api.createIntegration(this.appName, 'ifttt', {
        key: this.ifttt.key
      })
    },
    async deleteIftttIntegration() {
      await api.deleteIntegration(this.appName, 'ifttt')
    },
    async createThingSpeakIntegration() {
      await api.createIntegration(this.appName, 'thing_speak', {
        write_api_key: this.thingSpeak.write_api_key
      })
    },
    async deleteThingSpeakIntegration() {
      await api.deleteIntegration(this.appName, 'thing_speak')
    },
    async createOutgoingWebhookIntegration() {
      await api.createIntegration(this.appName, 'outgoing_webhook', {
        webhook_url: this.outgoingWebhook.url
      })
    },
    async deleteOutgoingWebhookIntegration() {
      await api.deleteIntegration(this.appName, 'outgoing_webhook')
    }
  },

  async mounted() {
    this.app = await api.getApp(this.appName)

    const integrations = await api.getIntegrations(this.appName)
    for (const integration of integrations) {
      const config = JSON.parse(integration.config)
      switch (integration.service) {
        case 'ifttt':
          this.ifttt.activated = true;
          this.ifttt.key = config.key;
          break;
        case 'thing_speak':
          this.thingSpeak.activated = true;
          this.thingSpeak.write_api_key = config.write_api_key;
          break;
        case 'outgoing_webhook':
          this.outgoingWebhook.activated = true;
          this.outgoingWebhook.url = config.webhook_url;
          break;
        case 'icoming_webhook':
          this.incomingWebhook.activated = true;
          this.incomingWebhook.key = integration.token;
          break;
      }
    }
  }
};
</script>

<style lang="scss">
.integration-status {
  font-weight: 700;
  color: var(--fg1-color);

  &.activated {
    color: var(--positive-color);
  }
}
</style>
