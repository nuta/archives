<template>
  <simple-layout :class="[theme + '-theme']">
    <header>
      <h1>MakeStack</h1>
      <p>A "batteries-included" IoT PaaS for super-rapid prototyping.</p>
    </header>
    <main>
      <form @submit.prevent="login">

        <div class="field">
          <label>Username</label>
          <input type="text" v-model="username" placeholder="Username" required="required" autofocus>
        </div>

        <div class="field">
          <label>Password</label>
          <input type="password" v-model="password" placeholder="Password" required="required">
        </div>

        <details>
          <summary>Server URL (defaults to {{ serverUrl }})</summary>
          <div class="content">
            <label>Server URL</label>
            <input type="text" v-model="serverUrl" placeholder="Server URL" required="required" autofocus>
          </div>
        </details>

        <div class="field">
          <div class="theme-switcher">
            <div :class="{ active: theme == 'simple' }" class="simple-theme theme-button"
             @click="saveTheme('simple')"
             @mouseover="previewTheme('simple')" @mouseleave="previewTheme(savedTheme)"></div>

            <div :class="{ active: theme == 'monokai' }" class="monokai-theme theme-button"
             @click="saveTheme('monokai')"
             @mouseover="previewTheme('monokai')" @mouseleave="previewTheme(savedTheme)"></div>

            <div :class="{ active: theme == 'solarized-light' }" class="solarized-light-theme theme-button"
             @click="saveTheme('solarized-light')"
             @mouseover="previewTheme('solarized-light')" @mouseleave="previewTheme(savedTheme)"></div>
          </div>
        </div>

        <div class="action">
          <input type="submit" value="Login" class="primary">
        </div>

        <div class="action">
          <p>
            <nuxt-link to="/reset-password">Reset Password</nuxt-link>
            <br>
            <nuxt-link to="/create-account">New to MakeStack? Create your account!</nuxt-link>
          </p>
        </div>
      </form>
    </main>
  </simple-layout>
</template>

<script>
import api from "~/assets/js/api";
import { getCurrentTheme, setTheme } from "~/assets/js/preferences";
import SimpleLayout from "~/components/simple-layout";

export default {
  components: { SimpleLayout },
  data: () => {
    return {
      serverUrl: DEFAULT_SERVER_URL,
      username: "",
      password: "",
      theme: getCurrentTheme(),
      savedTheme: getCurrentTheme()
    };
  },
  methods: {
    async login() {
      await api.login(this.serverUrl, this.username, this.password)
      this.$router.push({ path: 'apps' })
    },
    previewTheme(theme) {
      this.theme = theme
    },
    saveTheme(theme) {
      this.theme = theme
      this.savedTheme = theme
      setTheme(theme)
    }
  },
  mounted() {
    const params = new URLSearchParams(window.location.search)
    if (params.get('account_confirmation_success') === 'true') {
      this.$Notification.success('Verified your email address. Thank you!')
    }
  },
  beforeMount() {
    if (api.loggedIn()) {
      this.$router.replace({ path: 'apps' })
    }
  }
}
</script>

<style lang="scss" scoped>
@import "~assets/css/theme";
.theme-switcher {
  border-radius: 10px;
  padding: 20px 30px;
  width: 200px;
  margin: 0 auto;
  display: flex;
  justify-content: space-between;

  .theme-button {
    width: 30px;
    height: 30px;
    background: var(--bg0-color);
    border-radius: 50%;
    border: 1px solid var(--border-color);

    &:hover {
      cursor: pointer;
    }
  }
}
</style>

