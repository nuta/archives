<template>
<app-layout :app-name="appName">
  <h3>Create a new integration</h3>
  <form v-on:submit.prevent="create">
    <div class="uk-margin">
      <label class="uk-form-label">Service</label>
      <div class="uk-form-controls">
        <select v-model="newIntegration.service" class="uk-select uk-form-width-large">
          <template v-for="(title, value) in supportedServices">
            <option :value="value">{{ title }}</option>
          </template>
        </select>
      </div>
    </div>

    <div class="uk-margin">
      <div v-if="newIntegration.service == 'outgoing_webhook'">
        <label class="uk-form-label">Outgoing Webhook URL</label>
        <div class="uk-form-controls">
          <input type="url" v-model="newIntegration.webhook_url" class="uk-input uk-form-width-large" placeholder="Webhook URL" required>
        </div>
      </div>

      <div v-if="newIntegration.service == 'incoming_webhook'">
        We will generate a webhook URL just for you!
      </div>

      <div v-if="newIntegration.service == 'ifttt'">
        <label class="uk-form-label">IFTTT Key</label>
        <div class="uk-form-controls">
          <input type="text" v-model="newIntegration.ifttt_key" class="uk-input uk-form-width-large" placeholder="IFTTT Key" required>
        </div>
      </div>

      <div v-if="newIntegration.service == 'slack'">
        <label class="uk-form-label">Slack Webhook URL</label>
        <div class="uk-form-controls">
          <input type="text" v-model="newIntegration.slack_webhook_url"
           class="uk-input uk-form-width-large" placeholder="Webhook URL" required>
        </div>
      </div>

      <div v-if="newIntegration.service == 'datadog'">
        <label class="uk-form-label">Datadog API Key</label>
        <div class="uk-form-controls">
          <input type="text" v-model="newIntegration.datadog_api_key"
           class="uk-input uk-form-width-large" placeholder="Datadog API Key" required>
        </div>
      </div>

      <div v-if="newIntegration.service == 'sakuraio'">
        <label class="uk-form-label">sakura.io Incoming Webhook Token</label>
        <div class="uk-form-controls">
          <input type="text" v-model="newIntegration.sakuraio_incoming_webhook_token"
           class="uk-input uk-form-width-large" placeholder="aaaaaaaa-1111-2222-3333-abcdef1234567" required>
        </div>
      </div>
    </div>

    <div class="uk-margin">
      <label class="uk-form-label">Call webhook URL when:</label>
      <div class="uk-form-controls">
        <div class="uk-form-controls">
          <input type="checkbox" v-model="newIntegration.on_event">
          a device sent events
        </div>
        <div class="uk-form-controls">
          <input type="checkbox" v-model="newIntegration.on_device_change">
          a device changes its state
        </div>
      </div>
    </div>

    <div class="uk-margin">
      <label class="uk-form-label">Comment (Optional)</label>
      <div class="uk-form-controls">
        <input type="text" v-model="newIntegration.comment"
         class="uk-input uk-form-width-large" placeholder="Comment (Optional)">
  　　  </div>
    </div>

    <input type="submit" class="uk-button uk-button-primary uk-margin-small-top" value="Add an integration">
  </form>
</app-layout>
</template>

<script>
import api from "js/api";
import AppLayout from "layouts/app";

export default {
  components: { AppLayout },
  data() {
    return {
      appName: app.$router.currentRoute.params.appName,
      supportedServices: {
        outgoing_webhook: "Outgoing Webhook",
        ifttt: "IFTTT",
        slack: "Slack",
        datadog: "Datadog",
        incoming_webhook: "Incoming Webhook",
        sakuraio: "sakura.io"
      },
      newIntegration: {
        service: "outgoing_webhook",
        comment: "",
        webhook_url: "",
        incoming_webhook_token: "",
        sakuraio_incoming_webhook_token: "",
        slack_webhook_url: "",
        datadog_api_key: "",
        ifttt_key: "",
        on_event: true,
        on_device_change: true
      }
    };
  },
  methods: {
    createConfigForServer(form) {
      let config = {
        on_event: form.on_event,
        on_device_change: form.on_device_change
      }

      switch (form.service) {
        case "outgoing_webhook":
          config['webhook_url'] = form.webhook_url
          break;

        case "ifttt":
          config['key'] = form.ifttt_key
          break;

        case "slack":
          config['webhook_url'] = form.slack_webhook_url
          break

        case "datadog":
          config['api_key'] = form.datadog_api_key
          break

        case "sakuraio":
          config['incoming_webhook_token'] = form.sakuraio_incoming_webhook_token
          break

        case "incoming_webhook":
          break
      }

      return JSON.stringify(config);
    },

    async create() {
      await api.createIntegration(
        this.appName,
        this.newIntegration.service,
        this.createConfigForServer(this.newIntegration),
        this.newIntegration.comment)

      this.$Notification.success('Created a new integration.')
      this.$router.push({ name: 'integrations', params: { appName: this.appName } })
    },
  },
  mounted() {
    this.$Progress.finish()
  }
};
</script>

<style lang="scss" scoped>
.integrations {
  form {
    background: #fafafa;
    width: 500px;
    padding: 10px;
    border-radius: 5px;
  }
}
</style>
