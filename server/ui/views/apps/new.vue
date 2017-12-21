<template>
<dashboard-layout title="Create a new app">
  <form @submit.prevent="createApp">
    <div class="uk-margin">
      <label class="uk-form-label">Name</label>
      <div class="uk-form-controls">
        <input type="text" v-model="appName" class="uk-input uk-form-width-large" required="required" autofocus placeholder="Name">
      </div>
    </div>

    <div class="uk-margin">
      <label class="uk-form-label">API</label>
        <div class="uk-form-controls uk-form-controls-text">
          <label>
            <input type="radio" class="uk-radio" v-model="appAPI" value="nodejs">
            <b>Node.js:</b>
            A full Linux environment for super-rapid prototyping.</label>
        </div>
      </div>
    </div>

    <div class="uk-margin">
      <input type="submit" class="uk-button uk-button-primary uk-margin-xlarge-top" value="Create">
    </div>
  </form>
</dashboard-layout>
</template>

<script>
import api from "js/api"
import DashboardLayout from "layouts/dashboard"

export default {
  components: { DashboardLayout },
  data() {
    return {
      appName: '',
      appAPI: 'nodejs'
    };
  },
  methods: {
    async createApp() {
      await api.createApp(this.appName, this.appAPI)

      const templateFiles = [
        { path: 'app.js', body: '// app.js' },
        { path: 'app.yaml', body:  `{ "name": "${this.appName}", "private": true }` }
      ]

      for (const file of templateFiles) {
        await api.saveFile(this.appName, file.path, file.body)
      }

      this.$Notification.success('Created a new app.')
      this.$router.push({ name: "apps" })
    }
  },
  beforeMount() {
    this.$Progress.finish()
  }
}
</script>
