<template>
<dashboard-layout title="Integrations">
  <div class="integrations">
    <form v-for="integration in integrations">
      <fieldset>
        <div v-if="integration.service == 'outgoing_webhook'">
          <label>Outgoing Webhook URL</label>
          <input type="url" v-model="integration.webhook_url" placeholder="Webhook URL" required>
        </div>

        <div v-if="integration.service == 'incoming_webhook'">
          <label>Incoming Webhook Token</label>
          <input type="text" v-model="integration.incoming_webhook_token" readonly>
        </div>

        <div v-if="integration.service == 'ifttt'">
          <label>IFTTT Key</label>
          <input type="text" v-model="integration.ifttt_key" placeholder="IFTTT Key" required>
        </div>

        <div v-if="integration.service == 'slack'">
          <label>Slack Webhook URL</label>
          <input type="text" v-model="integration.slack_webhook_url"
          placeholder="https://hooks.slack.com/services/T00000000/B00000000/XXXXXXXXX" required>
        </div>

        <div v-if="integration.service == 'datadog'">
          <label>Datadog API Key</label>
          <input type="text" v-model="integration.datadog_api_key" placeholder="Datadog API Key" required>
        </div>

       <label>Call webhook URL when:</label>
        <p>
          <input type="checkbox" v-model="integration.on_event">
          a device sent events
        </p>
        <p>
          <input type="checkbox" v-model="integration.on_device_change">
          a device changes its state
        </p>

        <label>Comment (Optional)</label>
        <input type="text" v-model="integration.comment" placeholder="Comment (Optional)">
      </fieldset>

      <button v-on:click="update(integration)">Update</button>
      <button v-on:click="remove(integration)">Remove</button>
    </form>

    <form v-on:submit.prevent="create">
      <fieldset>
        <label>Service</label>
        <select v-model="newIntegration.service">
          <template v-for="(title, value) in supportedServices">
            <option :value="value">{{ title }}</option>
          </template>
        </select>

        <div v-if="newIntegration.service == 'outgoing_webhook'">
          <label>Outgoing Webhook URL</label>
          <input type="url" v-model="newIntegration.webhook_url" placeholder="Webhook URL" required>
        </div>

        <div v-if="newIntegration.service == 'incoming_webhook'">
          Just click Create!
        </div>

        <div v-if="newIntegration.service == 'ifttt'">
          <label>IFTTT Key</label>
          <input type="text" v-model="newIntegration.ifttt_key" placeholder="IFTTT Key" required>
          <label>Call webhook URL when:</label>
        </div>

        <div v-if="newIntegration.service == 'slack'">
          <label>Slack Webhook URL</label>
          <input type="text" v-model="newIntegration.slack_webhook_url"
          placeholder="Webhook URL" required>
        </div>

        <div v-if="newIntegration.service == 'datadog'">
          <label>Datadog API Key</label>
          <input type="text" v-model="newIntegration.datadog_api_key" placeholder="Datadog API Key" required>
        </div>

        <label>Call webhook URL when:</label>
        <p>
          <input type="checkbox" v-model="newIntegration.on_event">
          a device sent events
        </p>
        <p>
          <input type="checkbox" v-model="newIntegration.on_device_change">
          a device changes its state
        </p>

        <label>Comment (Optional)</label>
        <input type="text" v-model="newIntegration.comment" placeholder="Comment (Optional)">
      </fieldset>

      <input type="submit" value="Add an integration">
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
      apps: [],
      integrations: [],
      supportedServices: {
        outgoing_webhook: "Outgoing Webhook",
        ifttt: "IFTTT",
        slack: "Slack",
        datadog: "Datadog",
        incoming_webhook: "Incoming Webhook"
      },
      newIntegration: {
        service: "outgoing_webhook",
        comment: "",
        webhook_url: "",
        incoming_webhook_token: "",
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

        case "incoming_webhook":
          break
      }

      return JSON.stringify(config);
    },
    create() {

      api.createIntegration(
        this.appName,
        this.newIntegration.service,
        this.createConfigForServer(this.newIntegration),
        this.newIntegration.comment);
    },
    update(integration) {

      api.updateIntegration(
        this.appName,
        integration.service,
        this.createConfigForServer(integration),
        integration.comment);
    },
    remove(integration) {
      api.deleteIntegration(this.appName, integration.service)
    }
  },
  beforeMount() {
    api.getIntegrations(this.appName).then(r => {
      this.integrations = r.json.map(integration => {
        let config = JSON.parse(integration.config);
        return {
          service: integration.service,
          comment: integration.comment,
          webhook_url: config.webhook_url || "",
          incoming_webhook_token: integration.token || "",
          slack_webhook_url: config.webhook_url || "",
          ifttt_key: config.key || "",
          datadog_api_key: config.api_key || "",
          on_event: config.on_event || false,
          on_device_change: config.on_device_change || false
        };
      });
    });
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
