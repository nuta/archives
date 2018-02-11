<template>
  <div id="app" :class="[platform]">
    <notifications :notifications="notifications"></notifications>
    <nuxt></nuxt>
  </div>
</template>

<script>
import { getCurrentTheme } from "~/assets/js/preferences"
import Notifications from "~/components/notifications"
import { setTimeout, clearTimeout } from 'timers';

export default {
  components: { Notifications },
  data() {
    return {
      platform: PLATFORM,
      notifications: []
    }
  },
  methods: {
    handleError(error) {
      if (this.clearTimer) {
        clearTimeout(this.clearTimer)
      }

      this.notifications.push({
        title: error.message,
        type: 'danger'
      })

      this.clearTimer = setTimeout(() => this.notifications = [], 5000)
    }
  },
  beforeCreate() {
    this.theme = getCurrentTheme()
    document.body.classList.add(this.theme + '-theme')
  },
  mounted() {
    window.addEventListener("unhandledrejection", (error) => this.handleError(error.reason))
    window.onerror = (message, source, lineno, colno, error) => this.handleError(error)
  },
  beforeDestroy() {
    window.removeEventListener("unhandledrejection", this.handleError)
  }
}
</script>

<style lang="scss">
@import "~assets/css/default";
@import "~assets/css/containers";
@import "~assets/css/theme";
@import "~assets/css/form";
@import "~assets/css/table";

body {
  // Theme Switcher in the login page changes background-color.
  transition: background-color 0.5s ease;
  background-color: var(--bg0-color);
}

#app {
  font-family: 'Open Sans', -apple-system, BlinkMacSystemFont, 'Segoe UI', Ubuntu, 'Helvetica Neue', sans-serif;
  box-sizing: border-box;
}
</style>
