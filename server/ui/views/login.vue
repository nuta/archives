<template>
<wizard-layout title="Login">
  <form @submit.prevent="login">
    <div class="uk-margin">
      <label class="uk-form-label">Username</label>
      <div class="uk-form-controls">
        <input type="text" v-model="username" placeholder="Username" class="uk-input uk-form-width-large" required="required" autofocus>
      </div>
    </div>

    <div class="uk-margin">
      <label class="uk-form-label">Password</label>
      <div class="uk-form-controls">
        <input type="password" v-model="password" placeholder="Password" class="uk-input uk-form-width-large" required="required" autofocus>
      </div>
    </div>

    <div class="uk-text-center">
      <input type="submit" value="Login" class="uk-button uk-button-primary uk-margin-large-top">
    </div>

    <div class="uk-text-center uk-margin-small-top">
      <router-link :to="{ name: 'reset-password' }" class="uk-link-text">Reset Password</router-link>
    </div>
  </form>
</wizard-layout>
</template>


<script>
import api from "js/api";
import WizardLayout from "layouts/wizard";

export default {
  components: { WizardLayout },
  data: () => {
    return {
      username: "",
      password: ""
    };
  },
  methods: {
    async login() {
      await api.login(this.username, this.password)
      this.$router.push("/apps")
      this.$Progress.finish()
    }
  },
  mounted() {
    const params = new URLSearchParams(window.location.search)
    if (params.get('account_confirmation_success') === 'true') {
      this.$Notification.success('Verified your email address. Thank you!')
    }

    this.$Progress.finish()
  }
}
</script>
