<template>
<dashboard-layout title="TODO">
  <div class="list">
    <div v-for="app in apps" class="element">
      <div class="title">
        <router-link :to="{ name: 'code', params: { appName: app.name }}">
          {{ app.name }}
        </router-link>
      </div>

      <div class="rightside">
        <table class="metadata">
          <tr>
             <td><i class="fa fa-globe" aria-hidden="true"></i>API</td>
             <td>{{ app.api | describeAPI }}</td>
          </tr>
        </table>
      </div>
    </div>
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
      apps: []
    };
  },
  beforeMount() {
    api.getApps().then(r => {
      this.apps = r.json;
    });
  }
};
</script>

<style lang="scss" scoped>
</style>
