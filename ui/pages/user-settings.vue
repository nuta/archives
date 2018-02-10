<template>
  <simple-layout title="User Settings">
    <header>
      <h1>User Settings</h1>
      <nuxt-link :to="{ name: 'apps' }" class="return-link">
        <i class="fas fa-chevron-left"></i>
        Return to app
      </nuxt-link>
    </header>

    <main>
      <div class="sections">
        <section>
          <header>
            <h1>Change Email Address</h1>
          </header>
          <main>
            <form @submit.prevent="changeEmailAddress">
              <div class="field">
                <label>New Email Address</label>
                <input type="text" v-model="newEmailAddress" placeholder="New Email Address">
              </div>
              <input type="submit" value="Change Email Address" class="primary">
            </form>
          </main>
        </section>
        <section>
          <header>
            <h1>Change Password</h1>
          </header>
          <main>
            <form @submit.prevent="changePassword">
              <div class="field">
                <label>Current Password</label>
                <input type="text" v-model="newPassword" placeholder="Current Password">
              </div>
              <div class="field">
                <label>New Password</label>
                <input type="text" v-model="newPassword" placeholder="New Password">
              </div>
              <div class="field">
                <label>Current Password (Confirmation)</label>
                <input type="text" v-model="retypedNewPassword" placeholder="Retype New Password">
              </div>
              <input type="submit" value="Change Password" class="primary">
            </form>
          </main>
        </section>
        <section>
          <header>
            <h1>Delete Account</h1>
          </header>
          <main>
            <button @click="deleteAccount" class="danger">
              Delete {{ username }}
            </button>
          </main>
        </section>
      </div>
    </main>
  </simple-layout>
</template>

<script>
import api from "~/assets/js/api"
import SimpleLayout from "~/components/simple-layout"

export default {
  components: { SimpleLayout },
  data() {
    return {
      username: api.username,
      email: api.email,
      currentPassword: '',
      newPassword: '',
      retypedNewPassword: '',
      newEmailAddress: ''
    }
  },
  methods: {
    async changeEmailAddress() {
      await api.updateUser({ email: this.newEmailAddress })
    },
    async changePassword() {
      await api.updateUser({
        current_password: this.currentPassword,
        password: this.newPassword,
        passwordConfirmation: this.retypedNewPassword
      })
    },
    async deleteAccount() {
      const typed = window.prompt('Are you sure you want to delete the account? Type username to continue.')
      if (typed === this.username) {
        await api.deleteUser()
      }
    }
  }
}
</script>
