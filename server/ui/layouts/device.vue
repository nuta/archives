<template>
<dashboard-layout :title="deviceName">
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
import DashboardLayout from "layouts/dashboard"

export default {
  props: [ 'deviceName', 'path' ],
  components: { DashboardLayout },
  data() {
    return {
      tabs: [
        {
          title: 'Settings',
          path: 'deviceSettings',
          icon: 'icon: settings',
          params: { deviceName: this.deviceName }
        },
        {
          title: 'Configs',
          path: 'deviceConfigs',
          icon: 'icon: database',
          params: { deviceName: this.deviceName }
        },
        {
          title: 'Log',
          path: 'deviceLog',
          icon: 'icon: list',
          params: { deviceName: this.deviceName }
        }
      ]
    }
  }
}
</script>
