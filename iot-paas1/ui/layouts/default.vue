<template>
  <div id="app" :class="[platform]">
    <notifications :notifications="notifications"></notifications>
    <nuxt></nuxt>
  </div>
</template>

<script>
import { appendToLog } from "platform";
import api from "~/assets/js/api";
import { getCurrentTheme } from "~/assets/js/preferences";
import Notifications from "~/components/notifications";
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
      appendToLog(error.stack + '\n\n')

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
    const allowedRoutes = ['login', 'logout', 'reset-password', 'create-account']
    if (!api.loggedIn() && !allowedRoutes.includes(this.$route.name)) {
      this.$router.replace({ path: 'login' })
    }

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
@import 'normalize.css';
@import 'balloon-css/balloon.min.css';
@import "~assets/css/default";
@import "~assets/css/containers";
@import "~assets/css/theme";
@import "~assets/css/form";
@import "~assets/css/table";

html {
  height: 100%;
}

body {
  // Theme Switcher in the login page changes background-color.
  background-color: var(--bg0-color);
  transition: background-color 0.5s ease;

  height: 100%;
}

#__layout {
  height: 100%;
}

#__nuxt {
  height: 100%;
}

#app {
  font-family: 'Open Sans', -apple-system, BlinkMacSystemFont, 'Segoe UI', Ubuntu, 'Helvetica Neue', sans-serif;
  box-sizing: border-box;
  height: 100%;
}
</style>
