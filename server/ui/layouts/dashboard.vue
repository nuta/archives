<template>
  <div class="dashboard">
    <nav class="uk-navbar-container" uk-navbar>
      <div class="uk-navbar-left">
        <a href="/" class="uk-navbar-item uk-logo">
          <img src="/logo.svg" class="logo">
          MakeStack
        </a>
      </div>

      <div class="uk-navbar-center">
        <ul class="uk-navbar-nav">
          <li>
            <router-link :to="{ name: 'apps' }">
              <span uk-icon="icon: nut"></span>
              <span class="path">Apps</span>
            </router-link>
          </li>

          <li>
            <router-link :to="{ name: 'devices' }">
              <span uk-icon="icon: copy"></span>
              <span class="path">Devices</span>
            </router-link>
          </li>

          <li>
            <router-link :to="{ name: 'settings' }">
              <span uk-icon="icon: user"></span>
              <span class="path">Settings</span>
            </router-link>
          </li>

          <li>
            <a href="/documentation">
              <span uk-icon="icon: lifesaver"></span>
              <span class="path">Documentation</span>
            </a>
          </li>
        </ul>
      </div>

      <div class="uk-navbar-right">
        <div class="uk-navbar-item">
          <img :src="avatarUrl" class="uk-border-circle" width="30" height="30">
        </div>
      </div>
      <div uk-dropdown>
          <ul class="uk-nav uk-dropdown-nav">
              <li><router-link :to="{ name: 'force-login' }">Logout</router-link></li>
          </ul>
      </div>
    </nav>

    <h1>{{ title }}</h1>
    <div class="uk-container">
      <main>
        <slot></slot>
      </main>
    </div>
  </div>
</template>

<script>
import api from "js/api";
import md5 from "blueimp-md5";

export default {
  props: ["title"],
  components: { },
  data() {
    return {
      appName: this.$router.currentRoute.params.appName,
      deviceName: this.$router.currentRoute.params.deviceName,
      username: api.user.username
    };
  },
  computed: {
    avatarUrl() {
      return "https://www.gravatar.com/avatar/" + md5(api.user.email) + "?s=20&d=mm";
    }
  },
}
</script>

<style lang="scss">
.dashboard {
  & > h1 {
    background: #f3f3f3;
    width: 100%;
    font-size: 40px;
    font-weight: 200;
    padding: 20px 30px;
    margin: 0 0 30px 0;
    border-bottom: 1px solid #d7d7d7;
  }
}

nav {
  background: linear-gradient(to left, #7474bf, #348ac7) !important;
  height: 50px;

  .uk-logo {
    color: #fefefe;
    font-family: Roboto;
    font-weight: 100;

    .logo {
      position: relative;
      bottom: 5px;
      width: 25px;
      stroke: #fefefe !important;
      margin-right: 5px;
    }

    &:hover {
      color: #fefefe;
    }
  }

  .uk-navbar-nav > li > a {
    font-size: 16px;
    font-weight: 300;
    color: #fefefe;
    text-transform: none;

    &:hover {
      color: #dadada;
    }

    .uk-icon {
      padding-right: 5px;
    }
  }
}

main {
  & > .uk-tab {
    margin-top: -25px;
  }

  .uk-button, .uk-tab > * > a {
    text-transform: none;
  }

  table {
    .actions {
      text-align: right;
      vertical-align: middle;
      padding: 0;

      & > a {
        &:not(:first-child) {
          margin-left: 15px;
        }

        button {
          padding: 0;
        }
      }
    }
  }
}
</style>
