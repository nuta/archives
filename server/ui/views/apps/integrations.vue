<template>
<dashboard-layout title="Integrations">
  <div class="integrations">
    <form v-for="integration in integrations" v-on:submit.prevent="update(integration)">
      <fieldset>
        <div v-if="integration.service == 'outgoingWebhook'">
          <label>Webhook URL</label>
          <input type="url" v-model="integration.webhookUrl" placeholder="Webhook URL" required>
          <label>Call webhook URL when:</label>
          <p>
            <input type="checkbox" v-model="integration.onEvent">
            a device sent events
          </p>
          <p>
            <input type="checkbox" v-model="integration.onDeviceChange">
            a device changes its state
          </p>
        </div>
        <label>Comment (Optional)</label>
        <input type="text" v-model="integration.comment" placeholder="Comment (Optional)">
      </fieldset>

      <input type="submit" value="Update">
    </form>

    <form v-on:submit.prevent="create">
      <fieldset>
        <label>Service</label>
        <select v-model="newIntegration.service">
          <template v-for="(title, value) in supportedServices">
            <option :value="value">{{ title }}</option>
          </template>
        </select>
        <div v-if="newIntegration.service == 'outgoingWebhook'">
          <label>Webhook URL</label>
          <input type="url" v-model="newIntegration.webhookUrl" placeholder="Webhook URL" required>
          <label>Call webhook URL when:</label>
          <p>
            <input type="checkbox" v-model="newIntegration.onEvent">
            a device sent events
          </p>
          <p>
            <input type="checkbox" v-model="newIntegration.onDeviceChange">
            a device changes its state
          </p>
        </div>
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
      newone: { service: "webhook", config: {}, comment: "" },
      integrations: [],
      supportedServices: {
        outgoingWebhook: "Outgoing Webhook"
      },
      newIntegration: {
        service: "outgoingWebhook",
        comment: "",
        webhookUrl: "",
        onEvent: true,
        onDeviceChange: true
      }
    };
  },
  methods: {
    createConfigFromForm(form) {
      let config;

      switch (form.service) {
        case "outgoingWebhook":
          config = {
            webhookUrl: form.webhookUrl,
            onEvent: form.onEvent,
            onDeviceChange: form.onDeviceChange
          };
          break;

      }

      return JSON.stringify(config);
    },
    create() {
      
      api.createIntegration(
        this.appName,
        this.newIntegration.service,
        this.createConfigFromForm(this.newIntegration),
        this.newIntegration.comment);
    },
    update(integration) {

      api.updateIntegration(
        this.appName,
        integration.service,
        this.createConfigFromForm(integration),
        integration.comment);
    }
  },
  beforeMount() {
    api.getIntegrations(this.appName).then(r => {
      this.integrations = r.json.map(integration => {
        let config = JSON.parse(integration.config);
        return {
          service: integration.service,
          comment: integration.comment,
          webhookUrl: config.webhookUrl || "",
          onEvent: config.onEvent || false,
          onDeviceChange: config.onDeviceChange || false
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
