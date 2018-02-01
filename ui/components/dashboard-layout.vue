<template>
  <div class="dashboard-layout container">
    <header>
      <h1>{{ title }}</h1>
      <div class="hamburger">
        <img class="avatar" :src="avatarUrl">
      </div>
    </header>

    <main>
      <slot></slot>
    </main>
  </div>
</template>

<script>
import api from "~/assets/js/api"
import md5 from "blueimp-md5"

export default {
  props: ['title'],
  computed: {
      avatarUrl() {
        return "https://www.gravatar.com/avatar/" + md5(api.email) + "?s=30&d=mm";
    }
  },
  beforeCreate() {
    if (!api.loggedIn()) {
      this.$router.push({name: 'login'})
    }
  }
}
</script>


<style lang="scss" scoped>
@import "~assets/css/theme";

.dashboard-layout {
  max-width: 1200px;
  margin: 5px auto;
  padding: 10px 20px;

  header {
    width: 100%;
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding-bottom: 10px;
    border-bottom: 1px solid #eaeaea;

    & > * {
      width: 300px;
    }

    h1 {
      font-family: "Roboto", sans-serif;
      font-weight: 900;
      font-size: 25px;
      display: inline-block;
      margin: 0;
      padding: 0;
    }

    .breadcrumbs {
      text-align: center;
      list-style-type: none;
      margin: 0;
      margin-bottom: 5px;
      padding: 0;

      li {
        display: inline-block;

        a {
          color: $font-color;
          margin: 0 6px;
        }
      }

      &.with-slashes li:not(:last-child) {
        &:after {
          content: '/'
        }
      }
    }

    .hamburger {
      text-align: right;

      .avatar {
        border-radius: 15px;
      }
    }
  }

  main {
    margin-top: 50px;

    .actions {
      margin: 5px 0;

      & > * {
        &:not(:first-child) {
          margin-left: 8px;
        }
      }
    }

    section {
      &:not(:first-child) {
        margin-top: 30px;
      }

      h1 {
        font-family: 'Roboto';
        font-size: 22px;
        margin: 0;
      }

      .content {
        margin-top: 20px;
      }
    }
  }
}
</style>
