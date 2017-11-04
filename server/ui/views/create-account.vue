<template>
<wizard-layout title="Create an account">
  <form @submit.prevent="createAccount">
    <div class="uk-margin">
      <label class="uk-form-label">Username</label>
      <div class="uk-form-controls">
        <input type="text" v-model="username" placeholder="Username" class="uk-input uk-form-width-large" required="required" autofocus>
      </div>
    </div>

    <div class="uk-margin">
      <label class="uk-form-label">Email Address</label>
      <div class="uk-form-controls">
        <input type="text" v-model="email" placeholder="Email Address" class="uk-input uk-form-width-large" required="required">
      </div>
    </div>

    <div class="uk-margin">
      <label class="uk-form-label">Password</label>
      <div class="uk-form-controls">
        <input type="password" v-model="password" placeholder="Password" class="uk-input uk-form-width-large" required="required">
      </div>
    </div>

    <div class="uk-margin">
      <div class="uk-form-controls">
      <label class="uk-form-label">Password (confirmation)</label>
        <input type="password" v-model="passwordConfirmation" placeholder="Password (confirmation)" class="uk-input uk-form-width-large" required="required">
      </div>
    </div>

    <div class="uk-text-algin-center">
      <input type="submit" value="Create Account" class="uk-button uk-button-primary uk-margin-large-top">
    </div>
  </form>
</wizard-layout>
</template>


<script>
import api from "js/api"
import WizardLayout from "layouts/wizard"

export default {
  components: { WizardLayout },
  data: () => {
    return {
      username: "",
      email: "",
      password: "",
      passwordConfirmation: ""
    };
  },
  methods: {
    async createAccount() {
      await api.createUser(this.username, this.email, this.password, this.passwordConfirmation)
      this.$Notification.success('Sent an activation e-mail.')
      this.$router.push("/force-login")
    }
  },
  mounted() {
    this.$Progress.finish()
  }
}
</script>
