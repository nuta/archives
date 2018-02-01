<template>
  <login-layout>
    <form @submit.prevent="login">
      <div class="field">
        <label>Username</label>
        <input type="text" v-model="username" placeholder="Username" required="required" autofocus>
      </div>

      <div class="field">
        <label>Password</label>
        <input type="password" v-model="password" placeholder="Password" required="required">
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
  </login-layout>
</template>

<script>
import api from "~/assets/js/api";
import LoginLayout from "~/components/login-layout";

export default {
  components: { LoginLayout },
  data: () => {
    return {
      username: "",
      password: ""
    };
  },
  methods: {
    async login() {
      await api.login(this.username, this.password)
      this.$router.push({ path: 'apps' })
      this.$Progress.finish()
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
</style>
