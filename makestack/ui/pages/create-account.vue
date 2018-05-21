<template>
  <simple-layout>
    <header>
      <h1>Create an Account</h1>
      <nuxt-link :to="{ name: 'login' }" class="return-link">
        <i class="fas fa-chevron-left"></i>
        Login
      </nuxt-link>
    </header>
    <main>
      <form @submit.prevent="createAccount">
        <div class="field">
          <label>User Name</label>
          <input type="text" v-model="username" placeholder="username" required="required" autofocus>
        </div>

        <div class="field">
          <label>Email Address</label>
          <input type="text" v-model="email" placeholder="Email Address" required="required">
        </div>

        <div class="field">
          <label>Password</label>
          <input type="password" v-model="password" placeholder="Password" required="required">
        </div>

        <div class="field">
          <label>Password (Confirmation)</label>
          <input type="password" v-model="passwordConfirmation" placeholder="Password (Confirmation)" required="required">
        </div>

        <div class="field">
          <div ref="recaptcha"></div>
        </div>

        <div class="field">
         <p>
           By clicking "Create Account", you agree to our
           <a href="/tos.html" target="_blank">Term of Service</a> and <a href="/privacy-policy.html" target="_blank">Privacy Policy</a>.
          </p>
        </div>

        <div class="field">
          <input type="submit" :value="buttonMsg" class="primary">
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
      buttonMsg: 'Create Account',
      username: '',
      email: '',
      password: '',
      passwordConfirmation: ''
    };
  },
  methods: {
    async createAccount() {
      try {
        await api.createUser({
          server: location.origin,
          username: this.username,
          email: this.email,
          password: this.password,
          passwordConfirmation: this.passwordConfirmation,
          recaptcha: grecaptcha.getResponse(),
          agreeTos: 'yes'
        })
      } catch(e) {
        grecaptcha.reset()
        throw e
      }

      this.buttonMsg = 'Sent a confirmation email'
    }
  },
  mounted() {
    grecaptcha.render(this.$refs.recaptcha, { sitekey: RECAPTCHA_SITEKEY })
  }
}
</script>
