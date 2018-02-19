<template>
  <simple-layout>
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

        <details v-if="platform === 'desktop'">
          <summary>Server URL (defaults to {{ DEFAULT_SERVER_URL }})</summary>
          <div class="content">
            <label>Server URL</label>
            <input type="text" v-model="serverUrl" placeholder="Server URL" required="required" autofocus>
          </div>
        </details>

        <div class="field">
          <theme-switcher></theme-switcher>
        </div>

        <div class="action">
          <input type="submit" value="Login" class="primary">
        </div>

        <div class="action">
          <p>
            <nuxt-link :to="{ name: 'reset-password' }">Reset Password</nuxt-link>
            <br>
            <a href="#" v-if="platform === 'desktop'" @click.prevent="openMakeStackCloud">
              Create a account on MakeStack Cloud
            </a>
            <nuxt-link :to="{ name: 'create-account' }" v-else>
              New to MakeStack? Create your account!
            </nuxt-link>
          </p>
        </div>
      </form>
    </main>
  </simple-layout>
</template>

<script>
import api from "~/assets/js/api";
import SimpleLayout from "~/components/simple-layout";
import ThemeSwitcher from "~/components/theme-switcher";

export default {
  components: { SimpleLayout, ThemeSwitcher },
  data: () => {
    return {
      platform: PLATFORM,
      serverUrl: (PLATFORM === 'desktop') ? DEFAULT_SERVER_URL : location.origin,
      username: "",
      password: ""
    };
  },
  methods: {
    async login() {
      await api.login(this.serverUrl, this.username, this.password)
      this.$router.push({ path: 'apps' })
    },
    openMakeStackCloud() {
      window.openMakeStackCloud()
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
