<template>
  <dashboard-layout title="apps">
    <div class="row form-group" v-for="app in apps">
      <div class="col">
        <div class="card">
          <div class="card-body">
            <h3 class="card-title">{{ app.name }}</h3>
            <nuxt-link :to="{ name: 'apps-name', params: { name: app.name } }" class="btn btn-primary">Open</nuxt-link>
          </div>
        </div>
      </div>
    </div>
  </dashboard-layout>
</template>

<script>
import api from "~/assets/js/api"
import DashboardLayout from "~/components/dashboard-layout"

export default {
  components: { DashboardLayout },
  data() {
    return {
      apps: []
    }
  },
  async beforeMount() {
    this.apps = await api.getApps()
  }
}
</script>
