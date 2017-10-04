<template>
<dashboard-layout title="Deployments">
  <div class="list">
    <div v-for="deployment in deployments" class="element">
      <div class="title">
         {{ deployment.comment }}
      </div>

      <div class="rightside">
        <table class="metadata">
          <tr>
             <td><i class="fa fa-tag" aria-hidden="true"></i>Tag</td>
             <td>{{ deployment.tag }}</td>
          </tr>
          <tr>
             <td><i class="fa fa-calendar" aria-hidden="true"></i>Deployed at</td>
             <td>{{ deployment.createdAt }}</td>
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
      appName: app.$router.currentRoute.params.appName,
      deployments: []
    };
  },
  beforeMount() {
    api.getDeployments(this.appName)
      .then(r => this.deployments = r.json)
      .catch(error => notify("error", error));
  }
};
</script>
