<template>
<login-layout>
  <form v-on:submit.prevent="login">
    <fieldset>
      <label>Username</label>
      <input type="text" v-model="username" required="required" autofocus placeholder="Username">
      <label>Password</label>
      <input type="password" v-model="password" required="required" placeholder="Password">
    </fieldset>
    <input type="submit" value="Login">
  </form>
</login-layout>
</template>


<script>
import api from "js/api";
import LoginLayout from "layouts/login";

export default {
  components: { LoginLayout },
  data: () => {
    return {
      username: "",
      password: ""
    };
  },
  methods: {
    login() {
      api.login(this.username, this.password).then(() => {
        this.$router.push("/apps");
      }).catch((r) => {
        alert(r);
      });
    }
  }
};
</script>
