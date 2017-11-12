<template>
<div class="welcome">
  <header>
    MakeStack
  </header>

  <main>
    <div class="description-area">
      <h1>Welcome to {{ host }}</h1>
      <div class="description">
        {{ description }}
      </div>
   </div>

    <div class="buttons-area">
      <router-link :to="{ name: 'login' }" class="uk-button uk-button-primary">
        Login
      </router-link>
      <router-link :to="{ name: 'create-account' }" class="uk-button uk-button-primary">
        Create Account
      </router-link>
    </div>
  </main>
</div>
</template>

<script>
export default {
  data() {
    return {
      host: location.host,
      description: WELCOME_MESSAGE
    }
  },
  mounted() {
    this.$Progress.finish()
  }
}
</script>

<style lang="scss" scoped>
header {
  padding: 10px 0px;
  text-align: center;
  border-bottom: 1px solid #eaeaea;
}

main {
  width: 800px;
  margin: 100px auto;
  display: flex;
  justify-content: space-between;

  .description-area {
    h1 {
      font-weight: 600;
      font-size: 23px;
    }
  }

  .buttons-area {
    border: 1px solid #cecece;
    border-radius: 3px;
    background: #fafafa;
    padding: 20px;

    .uk-button{
      display: block;

      &:not(:first-child) {
        margin-top: 30px;
      }
    }
  }
}
</style>
