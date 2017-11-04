<template>
<wizard-layout title="Reset Password">
  <form @submit.prevent="resetPassword">
    <div class="uk-margin">
      <label class="uk-form-label">Email Address</label>
      <div class="uk-form-controls">
        <input type="text" v-model="email" placeholder="Email Address" class="uk-input uk-form-width-large" required="required">
      </div>
    </div>

    <div class="uk-text-algin-center">
      <input type="submit" value="Reset Password" class="uk-button uk-button-primary uk-margin-large-top">
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
    async resetPassword() {
      await api.resetPassword(this.email)
      this.$Notification.success('Sent an e-mail to reset the password.')
    }
  },
  mounted() {
    this.$Progress.finish()
  }
}
</script>
