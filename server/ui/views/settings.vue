<template>
<dashboard-layout title="User Settings">
  <div>
    <h3>Change Email Address</h3>
    <form @submit.prevent="changeEmailAddress">
      <div class="uk-margin">
        <label class="uk-form-label">New email address</label>
        <div class="uk-form-controls">
          <input type="text" v-model="newEmailAddress" class="uk-input uk-form-width-large"  placeholder="New email address">
        </div>
      </div>

      <div class="uk-margin">
        <input type="submit" class="uk-button uk-button-primary" value="Change Email Address">
      </div>
    </form>
  </div>

  <div class="uk-margin-xlarge-top">
    <h3>Change Password</h3>
    <form @submit.prevent="changePassword">
      <div class="uk-margin">
        <label class="uk-form-label">Current Password</label>
        <div class="uk-form-controls">
          <input type="password" v-model="currentPassword" class="uk-input uk-form-width-large"  placeholder="Current Password">
        </div>
      </div>

      <div class="uk-margin">
        <label class="uk-form-label">New Password</label>
        <div class="uk-form-controls">
          <input type="password" v-model="newPassword" class="uk-input uk-form-width-large"  placeholder="New Password">
        </div>
      </div>

      <div class="uk-margin">
        <label class="uk-form-label">Retype New Password</label>
        <div class="uk-form-controls">
          <input type="password" v-model="retypedNewPassword" class="uk-input uk-form-width-large"  placeholder="New Password">
        </div>
      </div>

      <div class="uk-margin">
        <input type="submit" class="uk-button uk-button-primary" value="Change Password">
      </div>
    </form>
  </div>

  <div class="uk-margin-xlarge-top">
    <h3>Delete Account</h3>
    <p>Be careful! You can't restore a deleted account!</p>
    <button class="uk-button uk-button-danger" uk-toggle="target: #delete-account-modal">
      Delete Your Account
    </button>
  </div>

  <div class="uk-flex-top" uk-modal id="delete-account-modal">
    <div class="uk-modal-dialog">
      <form>
        <div class="uk-modal-header">
          <button class="uk-modal-close-default" type="button" uk-close></button>
          <h2 class="uk-modal-title">Delete an account</h2>
        </div>
        <div class="uk-modal-body">
          Are you sure you want to delete your account? You can't revert the operation!
        </div>
        <div class="uk-modal-footer uk-text-right">
            <button uk-toggle="target: #delete-account-modal" class="uk-button uk-button-primary">
              Cancel
            </button>
            <button @click="deleteUser" class="uk-button uk-button-danger">
              Delete Your Account
            </button>
        </div>
      </form>
    </div>
  </div>

</dashboard-layout>
</template>

<script>
import api from "js/api"
import DashboardLayout from "layouts/dashboard"

export default {
  components: { DashboardLayout },
  data() {
    return {
      newEmailAddress: null,
      currentPassword: null,
      newPassword: null,
      retypedNewPassword: null
    };
  },
  methods: {
    async changeEmailAddress() {
      await api.updateUser({ email: this.newEmailAddress })
    },

    async deleteUser() {
      await api.deleteUser()
      this.$Notification.success('Deleted your account. See you again someday.')
      this.$router.push({ name: "force-login" })
    },

    async changePassword() {
      await api.updateUser({
        current_password: this.currentPassword,
        password: this.newPassword,
        passwordConfirmation: this.retypedNewPassword
      })
    }
  },
  async beforeMount() {
    this.$Progress.finish()
  }
};
</script>

<style lang="scss" scoped>
</style>
