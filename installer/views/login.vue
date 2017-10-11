<template>
<main>
  <form v-on:submit.prevent="login">
    <div>
      <label>Server URL</label>
      <input type="text" v-model="url" required="required" autofocus placeholder="URL">
    </div>
    <div>
      <label>Username</label>
      <input type="text" v-model="username" required="required" autofocus placeholder="Username">
    </div>
    <div>
      <label>Password</label>
      <input type="password" v-model="password" required="required" placeholder="Password">
    </div>
    <input type="submit" value="Login">
  </form>
</main>
</template>


<script>
import api from "renderer/api";

export default {
  data: () => {
    return {
      url: "http://localhost:8080",
      username: "",
      password: ""
    };
  },
  methods: {
    login() {
      api.login(this.url, this.username, this.password).then(() => {
        this.$router.push({ name: 'wizard' })
      }).catch((r) => {
        console.error(r)
        alert('something went wrong!')
      });
    }
  }
};
</script>
