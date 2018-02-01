<template>
  <h1>Index</h1>
</template>

<script>
import api from "~/assets/js/api";

export default {
  beforeMount() {
    if (api.loggedIn()) {
      this.$router.replace({ path: 'apps' })
    } else {
      this.$router.replace({ path: 'login' })
    }
  }
}
</script>
