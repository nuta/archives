<template>
<app-layout path="deployments" :app-name="appName">
  <remote-content :loading="loading" :content="deployments">
    <p slot="welcome-title">No deployments yet!</p>

    <table slot="content" class="uk-table uk-table-divider">
      <thead>
        <tr>
          <th>Version</th>
          <th>Comment</th>
          <th>Tag</th>
          <th>Deployed from</th>
          <th>Deployed at</th>
        </tr>
      </thead>
      <tbody>
        <tr v-for="deployment in deployments">
          <td>#{{ deployment.version }}</td>
          <td>{{ deployment.comment }}</td>
          <td>{{ deployment.tag }}</td>
          <td>{{ deployment.deployed_from }}</td>
          <td>{{ deployment.created_at }}</td>
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
      deployments: []
    };
  },
  async beforeMount() {
    const deployments = await api.getDeployments(this.appName)
    this.deployments = deployments.sort((a, b) => a.version < b.version)
    this.$Progress.finish()
    this.loading = false
  }
};
</script>
