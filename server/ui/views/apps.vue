<template>
<dashboard-layout title="Applications">
  <div v-if="apps.length === 0" class="welcome-card">
    <p class="title">You have no apps yet!</p>
    <p class="description">To get started, create your first app.</p>
    <router-link :to="{ name: 'newApp' }">
      <button class="green">
        Create New App
      </button>
    </router-link>
  </div>
  <div v-else>
    <section class="two-columns">
      <div class="leftside">
      </div>
      <div class="rightside">
        <router-link :to="{ name: 'newApp' }">
          <button class="green">
            Create New App
          </button>
        </router-link>
      </div>
    </section>
    <div class="list">
      <div v-for="app in apps" class="element">
        <div class="title">
          <router-link :to="{ name: 'code', params: { appName: app.name }}">
            {{ app.name }}
          </router-link>
          <div>
            <router-link class="button" :to="{ name: 'code', params: { appName: app.name }}">
              <i class="fa fa-pencil-square-o" aria-hidden="true"></i>
              Code
            </router-link>
            <router-link class="button" :to="{ name: 'appLog', params: { appName: app.name }}">
              <i class="fa fa-align-left" aria-hidden="true"></i>
              Log
            </router-link>
            <router-link class="button" :to="{ name: 'appStores', params: { appName: app.name }}">
              <i class="fa fa-database" aria-hidden="true"></i>
              Stores
            </router-link>
          </div>
        </div>
  
        <div class="rightside">
          <table class="metadata">
            <tr>
               <td><i class="fa fa-globe" aria-hidden="true"></i>API</td>
               <td>{{ app.api | describeAPI }}</td>
            </tr>
            <tr>
               <td><i class="fa fa-calendar" aria-hidden="true"></i>Created at</td>
               <td>{{ app.created_at }}</td>
            </tr>
          </table>
        </div>
      </div>
    </div>
  </div>
</dashboard-layout>
</template>

<script>
import api from "js/api";
import DashboardLayout from "layouts/dashboard";
import { describeAPI } from "js/filters";

export default {
  components: { DashboardLayout },
  data() {
    return {
      apps: []
    };
  },
  filters: { describeAPI },
  beforeMount() {
    api.getApps()
      .then(r => this.apps = r.json)
      .catch(error => notify("error", error));
  }
};
</script>

<style lang="scss" scoped>
</style>
