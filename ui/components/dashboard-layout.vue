<template>
  <div class="dashboard-layout">
    <header>
      <nav>
        <dropdown :items="navItems" :title-style="navTitleStyle" :selected="navSelected"></dropdown>
      </nav>
      <div class="hamburger">
        <dropdown :items="appSwitcherItems" :selected="appName" :title-style="appSwitcherTitleStyle"></dropdown>
        <div class="avatar-container" :data-balloon="avatarBalloon" data-balloon-pos="down-right">
          <img class="avatar" :src="avatarUrl">
        </div>
      </div>
    </header>

    <main :class="{ 'no-padding': noPadding }">
      <slot></slot>
    </main>
  </div>
</template>

<script>
import Dropdown from "~/components/dropdown"
import api from "~/assets/js/api"
import md5 from "blueimp-md5"

export default {
  components: { Dropdown },
  props: ['title', 'appName', 'no-padding'],
  data() {
    return {
      avatarBalloon: `Logged in as ${api.username}`,
      navTitleStyle: {
        'font-family': '"Roboto", sans-serif',
        'font-weight': 600,
        'font-size': '28px'
      },
      appSwitcherTitleStyle: {
        'font-weight': 600
      },
      navItems: [
        {
          title: 'Code',
          to: { name: 'apps-appName', params: { name: this.appName } }
        },
        {
          title: 'Devices',
          to: { name: 'apps-appName-devices', params: { name: this.appName } }
        },
        {
          title: 'Setup a Device',
          to: { name: 'apps-appName-setup-device', params: { name: this.appName } }
        },
        {
          title: 'Settings',
          to: { name: 'apps-appName-settings', params: { name: this.appName } },
        },
        {
          title: 'User Settings',
          to: { name: 'user-settings' },
          divider: true
        },
        {
          title: 'Logout',
          to: { name: 'logout' },
          divider: true
        }
      ],
      navSelected: this.title,
      appSwitcherItems: []
    }
  },
  computed: {
    avatarUrl() {
      return "https://www.gravatar.com/avatar/" + md5(api.email) + "?s=30&d=mm";
    }
  },
  async beforeMount() {
    const appItems = (await api.getApps()).map(app => {
      return {
        title: app.name,
        to: { name: 'apps-appName', params: { name: app.name } }
      }
    })

    this.appSwitcherItems = [
      ...appItems,
      {
        title: 'Create a new app',
        icon: 'plus',
        divider: true,
        bold: true,
        to: { name: 'create-app' }
      }
    ]
  },
  beforeCreate() {
    if (!api.loggedIn()) {
      this.$router.push({name: 'login'})
    }
  }
}
</script>


<style lang="scss" scoped>
.dashboard-layout {
  height: 100vh;
  background-color: var(--bg1-color);
  color: var(--fg0-color);

  & > header {
    height: 50px;
    display: flex;
    justify-content: space-between;
    align-items: center;
    padding: 10px 8px;
    background-color: var(--bg0-color);

    .hamburger {
      display: flex;
      justify-content: space-between;

      .avatar-container {
        width: fit-content;
        margin-left: 15px;
        cursor: default;
        .avatar {
          border-radius: 15px;
        }
      }
    }
  }

  & > main {
    padding: 15px 30px;
    &.no-padding {
      padding: 0;
    }
  }
}
</style>
