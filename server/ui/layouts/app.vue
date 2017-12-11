<template>
<dashboard-layout :title="appName">
  <ul uk-tab class="uk-flex-center">
    <template v-for="tab in tabs">
      <li :class="[path === tab.path ? 'uk-active' : '']">
        <router-link :to="{ name: tab.path, params: tab.params }">
          <span :uk-icon="tab.icon"></span>
          {{ tab.title }}
        </router-link>
      </li>
    </template>
  </ul>

  <slot></slot>
</dashboard-layout>
</template>

<script>
import DashboardLayout from "layouts/dashboard";

export default {
  props: [ 'appName', 'path' ],
  components: { DashboardLayout },
  data() {
    return {
      tabs: [
        {
          title: 'Code',
          path: 'code',
          icon: 'icon: code',
          params: { appName: this.appName }
        },
        {
          title: 'Configs',
          path: 'appConfigs',
          icon: 'icon: database',
          params: { appName: this.appName }
        },
        {
          title: 'Log',
          path: 'appLog',
          icon: 'icon: list',
          params: { appName: this.appName }
        },
        {
          title: 'Deployments',
          path: 'deployments',
          icon: 'icon: bolt',
          params: { appName: this.appName }
        },
        {
          title: 'Integrations',
          path: 'integrations',
          icon: 'icon: social',
          params: { appName: this.appName }
        },
        {
          title: 'Settings',
          path: 'appSettings',
          icon: 'icon: settings',
          params: { appName: this.appName }
        },
      ]
    }
  }
}
</script>
