<template>
  <login-layout>
    <form @submit.prevent="login">
      <div class="field">
        <label>Username</label>
        <input type="text" v-model="username" placeholder="Username" required="required" autofocus>
      </div>

      <div class="field">
        <label>Password</label>
        <input type="password" v-model="password" placeholder="Password" required="required">
      </div>

      <div class="action">
        <input type="submit" value="Login" class="primary">
      </div>

      <div class="action">
        <p>
          <nuxt-link to="/reset-password">Reset Password</nuxt-link>
          <br>
          <nuxt-link to="/create-account">New to MakeStack? Create your account!</nuxt-link>
        </p>
      </div>
    </form>
  </login-layout>
</template>

<script>
import api from "~/assets/js/api";
import LoginLayout from "~/components/login-layout";

export default {
  components: { LoginLayout },
  data: () => {
    return {
      username: "",
      password: ""
    };
  },
  methods: {
    async login() {
      await api.login('', this.username, this.password)
      this.$router.push({ path: 'apps' })
      this.$Progress.finish()
    }
  },
  mounted() {
    const params = new URLSearchParams(window.location.search)
    if (params.get('account_confirmation_success') === 'true') {
      this.$Notification.success('Verified your email address. Thank you!')
    }
  },
  beforeMount() {
    if (api.loggedIn()) {
      this.$router.replace({ path: 'apps' })
    }
  }
}
</script>

<style lang="scss" scoped>
@mixin button {
  border: none;
  border-radius: 5px;
  padding: 9px 12px;
  font-family: 'Open Sans';
  font-weight: 600;
  background: #f3f3f3;
  color: #434343;
  border-bottom: 5px solid #cacaca;
  font-size: 15px;

  i.fa {
    position: relative;
    bottom: 1px;
    padding-right: 3px;
    font-size: 16px;
  }

  &.simple {
    background: #ffffff;
    border: 1px solid #cacaca;
    padding: 7px;

    i.fa {
      padding: 0;
      margin: 0;
      bottom: 0;
    }
  }

  &:hover {
    cursor: pointer;
    transition: 0.2s ease-in-out;
    background: #f7f7f7;
  }

  &.primary {
    background: #1e9fe8;
    border-bottom-color: #1678ac;
    color: #ffffff;

    &:hover {
      background: #21afff;
    }
  }
}

button {
  @include button;
}

form {
  .field {
    &:not(first-child) {
      margin-top: 45px;
    }
  }

  .action {
    display: flex;
    justify-content: center;
    text-align: center;
    margin-top: 30px;

    p {
      font-weight: 600;
      line-height: 28px;
    }
  }

  label {
    font-weight: 600;
    display: block;
    margin-left: 2px;
    margin-bottom: 10px;
  }

  input {
    font-family: 'Open Sans';
  }

  input[type=text], input[type=password] {
    font-size: 18px;
    border: 3px solid #efefef;
    border-radius: 7px;
    padding: 15px;
    width: 100%;
    box-sizing: border-box;
  }

  input[type=submit] {
    @include button;
    font-size: 22px;
  }
}
</style>
