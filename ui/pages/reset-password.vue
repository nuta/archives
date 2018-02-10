<template>
  <simple-layout>
    <header>
      <h1>Reset Password</h1>
    </header>
    <main>
      <form @submit.prevent="resetPassword">
        <div class="field">
          <label>Email Address</label>
          <input type="text" v-model="email" placeholder="Email Address" required="required" autofocus>
        </div>

        <details>
          <summary>Server URL (defaults to {{ serverUrl }})</summary>
          <div class="content">
            <label>Server URL</label>
            <input type="text" v-model="serverUrl" placeholder="Server URL" required="required" autofocus>
          </div>
        </details>

        <div class="action">
          <input type="submit" :value="buttonMsg" class="primary">
        </div>

        <div class="action">
          <p>
            <nuxt-link to="/login">Login</nuxt-link>
            <br>
            <nuxt-link to="/create-account">Create an account</nuxt-link>
          </p>
        </div>
      </form>
    </main>
  </simple-layout>
</template>

<script>
import api from "~/assets/js/api";
import SimpleLayout from "~/components/simple-layout";

export default {
  components: { SimpleLayout },
  data: () => {
    return {
      serverUrl: DEFAULT_SERVER_URL,
      buttonMsg: 'Reset Password',
      email: ''
    };
  },
  methods: {
    async resetPassword() {
      await api.resetPassword(this.serverUrl, this.email)
      this.buttonMsg = 'Sent an email'
    }
  }
}
</script>
