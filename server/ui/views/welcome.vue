<template>
<div>
  <nav class="uk-navbar-container" uk-navbar>
    <div class="uk-header-left">
      <a href="/" class="uk-navbar-item uk-logo">
        MakeStack
      </a>
    </div>

    <div class="uk-navbar-right">
      <div class="uk-navbar-item">
        <router-link :to="{ name: 'create-account' }" class="uk-button uk-button-primary">
          Create Account
        </router-link>
      </div>

      <div class="uk-navbar-item">
        <router-link :to="{ name: 'login' }" class="uk-button uk-button-primary">
          Login
        </router-link>
      </div>
    </div>
  </nav>

  <main>
    <h2>Hello World!</h2>
  </main>
</div>
</template>

<script>
export default {
}
</script>

<style lang="scss">
</style>
