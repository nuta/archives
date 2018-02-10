<template>
  <p>Redirecting to <nuxt-link :to="{ name: 'login' }">Login Page</nuxt-link>.</p>
</template>

<script>
import api from "~/assets/js/api"

export default {
  beforeCreate() {
    api.forceLogin()
    this.$router.push({ name: 'login' })
  }
}
</script>
