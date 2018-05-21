<template>
  <div></div>
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
