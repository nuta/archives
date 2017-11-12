<template>
<app-layout path="integrations" :app-name="appName">

  <div class="uk-container">
    <router-link :to="{ name: 'newIntegration', params: { appName: this.appName }}">
      <button class="uk-button uk-button-primary uk-align-right">
        <span uk-icon="icon: plus"></span>
        Add an integration
      </button>
    </router-link>
  </div>

  <remote-content :loading="loading" :content="integrations">
    <p slot="welcome-title">Integrate with external awesome services!</p>

    <table slot="content" class="uk-table uk-table-divider">
      <thead>
        <tr>
          <th>Name</th>
          <th>Comment</th>
          <th>URL / API key</th>
          <th></th>
        </tr>
      </thead>
      <tbody>
        <tr v-for="integration in integrations">
          <td>{{ integration.service }}</td>
          <td>{{ integration.comment }}</td>
          <td v-if="integration.service == 'outgoing_webhook'">
            {{ integration.webhook_url }}
          </td>

          <td v-if="integration.service == 'incoming_webhook'">
            <code>{{ integration.incoming_webhook_token }}</code>
          </td>

          <td v-if="integration.service == 'ifttt'">
            <code>{{ integration.ifttt_key }}</code>
          </td>

          <td v-if="integration.service == 'slack'">
            <code>{{ integration.slack_webhook_url }}</code>
          </td>

          <td v-if="integration.service == 'sakuraio'">
            <code>{{ integration.incoming_webhook_token }}</code> (Incoming Webhook)
          </td>

          <td v-if="integration.service == 'datadog'">
            <code>{{ integration.datadog_api_key }}</code>
          </td>
          <td class="actions">
            <button v-on:click="remove(integration)" class="uk-button uk-button-danger">
              <i class="fa fa-trash" aria-hidden="true"></i>
              Remove
            </button>
          </td>
        </tr>
      </tbody>
    </table>
  </remote-content>
</app-layout>
</template>

<script>
import api from "js/api";
import RemoteContent from "components/remote-content";
import AppLayout from "layouts/app";

export default {
  components: { AppLayout, RemoteContent },
  data() {
    return {
      appName: app.$router.currentRoute.params.appName,
      loading: true,
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

    this.$Progress.finish()
    this.loading = false
  }
};
</script>
