<template>
<dashboard-layout title="Integrations">
  <div class="integrations">
    <div class="element" v-for="integration in integrations">
      <div class="left-column">
        <div class="header">
          <span class="title">{{ integration.service }}</span>
          <span class="description">{{ integration.comment }}</span>
        </div>

        <table class="fields">
          <td v-if="integration.service == 'outgoing_webhook'">
            <span class="name">Webhook URL</span>
            <span class="value">{{ integration.webhook_url }}</span>
          </td>

          <td v-if="integration.service == 'incoming_webhook'">
            <span class="name">Token</span>
            <span class="value">{{ integration.incoming_webhook_token }}</span>
          </td>

          <td v-if="integration.service == 'ifttt'">
            <span class="name">API Key</span>
            <span class="value">{{ integration.ifttt_key }}</span>
          </td>

          <td v-if="integration.service == 'slack'">
            <span class="name">Webhook URL</span>
            <span class="value">{{ integration.slack_webhook_url }}</span>
          </td>

          <td v-if="integration.service == 'datadog'">
            <span class="name">API Key</span>
            <span class="value">{{ integration.datadog_api_key }}</span>
          </td>
        </table>
      </div>

      <div class="right-column">
        <button v-on:click="remove(integration)">Remove</button>
      </div>
    </div>
  </div>

  <router-link :to="{ name: 'newIntegration', params: { appName: this.appName }}">
    <button> add an integration</button>
  </router-link>
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
      integrations: []
    };
  },
  methods: {
    remove(integration) {
      api.deleteIntegration(this.appName, integration.name)
    }
  },
  async beforeMount() {
    this.integrations = (await api.getIntegrations(this.appName)).map(integration => {
      let config = JSON.parse(integration.config);
      return {
        service: integration.service,
        comment: integration.comment,
        webhook_url: config.webhook_url,
        incoming_webhook_token: integration.token,
        slack_webhook_url: config.webhook_url,
        ifttt_key: config.key,
        datadog_api_key: config.api_key,
        on_event: config.on_event,
        on_device_change: config.on_device_change
      }
    })
  }
};
</script>

<style lang="scss" scoped>
.integrations {
  .element {
    padding: 16px;
    border: 1px solid #e8e8e8;
    border-radius: 3px;
    display: flex;
    justify-content: space-between;

    .title {
      font-size: 24px;
      font-weight: 700;
      display: inline-box;
    }

    .description {
      color: #777777;
      margin-left: 10px;
    }

    .fields {
      margin-top: 20px;

      .name {
        font-weight: 600;
      }

      .value {
        color: #7a7a7a;
      }
    }

    &:not(:first-child) {
      margin-top: 10px;
    }
  }
}
</style>
