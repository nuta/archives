<template>
<main-layout>
  <header>
    <h1>MakeStack Installer</h1>
  </header>

  <main>
    <form v-on:submit.prevent="login">
      <section>
        <label>Server URL</label>
        <input type="text" v-model="url" required="required" autofocus placeholder="URL">
      </section>
      <section>
        <label>Username</label>
        <input type="text" v-model="username" required="required" autofocus placeholder="Username">
      </section>
      <section>
        <label>Password</label>
        <input type="password" v-model="password" required="required" placeholder="Password">
      </section>

      <section>
        <input type="submit" :value="loginButtonMsg">
      </section>
    </form>
  </main>
</main-layout>
</template>


<script>
import MainLayout from "layouts/main"
import api from "renderer/api"

export default {
  components: { MainLayout },
  data: () => {
    return {
      url: "http://localhost:8080",
      username: "",
      password: "",
      loginButtonMsg: "Login"
    };
  },
  methods: {
    login() {
      this.loginButtonMsg = "Logging in..."
      api.login(this.url, this.username, this.password).then(() => {
        this.$router.push({ name: 'installer' })
      }).catch(e => {
        console.log(e)
        this.loginButtonMsg = "Login"
        alert(e.stack)
      })
    }
  }
};
</script>
