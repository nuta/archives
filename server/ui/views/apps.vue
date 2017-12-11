<template>
<dashboard-layout title="Applications">
  <router-link :to="{ name: 'newApp' }">
    <button class="uk-button uk-button-primary uk-align-right">
      <span uk-icon="icon: plus"></span>
      Create New App
    </button>
  </router-link>

  <remote-content :loading="loading" :content="apps">
    <p slot="welcome-title">You have no apps yet!</p>
    <p slot="welcome-description">To get started, create your first app.</p>

    <table slot="content" class="uk-table uk-table-divider">
      <thead>
        <tr>
          <th>Name</th>
          <th>API</th>
          <th>Created at</th>
          <th></th>
        </tr>
      </thead>
      <tbody>
        <tr v-for="app in apps">
          <td>
            <router-link :to="{ name: 'code', params: { appName: app.name }}">
              {{ app.name }}
            </router-link>
          </td>
          <td>{{ app.api | describeAPI }}</td>
          <td>{{ app.created_at }}</td>
          <td class="actions">
            <router-link :to="{ name: 'code', params: { appName: app.name }}">
              <button class="uk-button">
                <span uk-icon="icon: code"></span>
                Code
              </button>
            </router-link>
            <router-link :to="{ name: 'appConfigs', params: { appName: app.name }}">
              <button class="uk-button">
                <span uk-icon="icon: database"></span>
                Configs
              </button>
            </router-link>
            <router-link :to="{ name: 'appLog', params: { appName: app.name }}">
              <button class="uk-button">
                <span uk-icon="icon: list"></span>
                Log
              </button>
            </router-link>
          </td>
        </tr>
      </tbody>
    </table>
  </remote-content>
</dashboard-layout>
</template>

<script>
import api from "js/api";
import DashboardLayout from "layouts/dashboard";
import RemoteContent from "components/remote-content";
import { describeAPI } from "js/filters";

export default {
  components: { DashboardLayout, RemoteContent },
  data() {
    return {
      loading: true,
      apps: []
    };
  },
  filters: { describeAPI },
  async beforeMount() {
    this.apps = await api.getApps()
    this.$Progress.finish()
    this.loading = false
  }
}
</script>

<style lang="scss" scoped>
</style>
