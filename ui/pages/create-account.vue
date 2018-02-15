<template>
  <simple-layout>
    <header>
      <h1>Create an Account</h1>
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

        <div class="action">
         <p>By clicking "Create Account", you agree to our Term of Service and Privacy Policy.</p>
          <input type="submit" :value="buttonMsg" class="primary">
        </div>

        <div class="action">
          <p>
            <nuxt-link to="/login">Login</nuxt-link>
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
      buttonMsg: 'Create Account',
      username: '',
      email: '',
      password: '',
      passwordConfirmation: ''
    };
  },
  methods: {
    async createAccount() {
      await api.createUser({
        server: location.host,
        username: this.username,
        email: this.email,
        password: this.password,
        passwordConfirmation: this.passwordConfirmation,
        recaptcha: grecaptcha.getResponse(),
        agreeTos: 'yes'
      })

      this.buttonMsg = 'Sent a confirmation email!'
    }
  },
  mounted() {
    grecaptcha.render(this.$refs.recaptcha, { sitekey: RECAPTCHA_SITEKEY })
  }
}
</script>
