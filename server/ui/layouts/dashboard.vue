<template>
  <div class="dashboard">
    <div class="container">
      <nav>
        <div class="title">
          <h1>
            MakeStack
          </h1>
        </div>
        <div class="user">
          <img :src="avatarUrl" class="avatar">
          <span>{{ username }}</span>
        </div>
        <ul class="links">
          <router-link :to="{ name: 'home' }"><li>
            <i class="fa green fa-home fa-fw" aria-hidden="true"></i>
            <span>Home</span>
          </li></router-link>

          <router-link :to="{ name: 'apps' }"><li>
            <i class="fa red fa-diamond fa-fw" aria-hidden="true"></i>
            <span>Apps</span>
          </li></router-link>

          <div v-if="appName" class="sublinks">
            <i class="fa fa-caret-up" aria-hidden="true"></i>
            <p>{{ appName }}</p>
            <ul>
              <router-link :to="{ name: 'appOverview' }"><li>
                <i class="fa fa-home" aria-hidden="true"></i>
                <span>Overview</span>
              </li></router-link>
              <router-link :to="{ name: 'code' }"><li>
                <i class="fa fa-pencil" aria-hidden="true"></i>
                <span>Code</span>
              </li></router-link>
              <router-link :to="{ name: 'appStores' }"><li>
                <i class="fa fa-database" aria-hidden="true"></i>
                <span>Stores</span>
              </li></router-link>
              <router-link :to="{ name: 'deployments' }"><li>
                <i class="fa fa-rocket" aria-hidden="true"></i>
                <span>Deployments</span>
              </li></router-link>
              <router-link :to="{ name: 'appLog' }"><li>
                <i class="fa fa-file-text-o" aria-hidden="true"></i>
                <span>Log</span>
              </li></router-link>
              <router-link :to="{ name: 'integrations' }"><li>
                <i class="fa fa-plug" aria-hidden="true"></i>
                <span>Integrations</span>
              </li></router-link>
              <router-link :to="{ name: 'appSettings' }"><li>
                <i class="fa fa-sliders" aria-hidden="true"></i>
                <span>Settings</span>
              </li></router-link>
            </ul>
          </div>

          <router-link :to="{ name: 'devices' }"><li>
            <i class="fa yellow fa-clone fa-fw" aria-hidden="true"></i>
            <span>Devices</span>
          </li></router-link>

          <div v-if="deviceName" class="sublinks">
            <i class="fa fa-caret-up" aria-hidden="true"></i>
            <p>{{ deviceName }}</p>
            <ul>
              <router-link :to="{ name: 'deviceOverview' }"><li>
                <i class="fa fa-home" aria-hidden="true"></i>
                <span>Overview</span>
              </li></router-link>
              <router-link :to="{ name: 'deviceStores' }"><li>
                <i class="fa fa-database" aria-hidden="true"></i>
                <span>Stores</span>
              </li></router-link>
              <router-link :to="{ name: 'deviceLog' }"><li>
                <i class="fa fa-file-text-o" aria-hidden="true"></i>
                <span>Log</span>
              </li></router-link>
              <router-link :to="{ name: 'deviceSettings' }"><li>
                <i class="fa fa-sliders" aria-hidden="true"></i>
                <span>Settings</span>
              </li></router-link>
            </ul>
          </div>

          <router-link :to="{ name: 'settings' }"><li>
            <i class="fa blue fa-magic fa-fw" aria-hidden="true"></i>
            <span>Settings</span>
          </li></router-link>
        </ul>
      </nav>

      <div class="content">
        <header>
          <div class="leftside">
            <span v-if="appName" class="title">
              {{ appName }}
            </span>
          </div>
          <div class="center">
            <span class="title">{{ title }}</span>
          </div>
          <div class="rightside">
            <a href="/documentation" class="undecorated-link" alt="documentation"><i class="fa fa-question-circle" aria-hidden="true"></i></a>
          </div>
        </header>
        <main>
          <slot></slot>
        </main>
      </div>
    </div>

  <notification :level="notificationLevel" :message="notificationMessage"></notification>
  </div>
</template>

<script>
import api from "js/api";
import md5 from "blueimp-md5";
import Notification from "components/notification";

export default {
  props: ["title"],
  components: { Notification },
  data() {
    return {
      appName: this.$router.currentRoute.params.appName,
      deviceName: this.$router.currentRoute.params.deviceName,
      username: api.user.username,
      notificationLevel: "",
      notificationMessage: ""
    };
  },
  computed: {
    avatarUrl() {
      return "https://www.gravatar.com/avatar/" + md5(api.user.email) + "?s=20&d=mm";
    }
  },
  beforeMount() {
    // XXX
    window.notify = (level, message) => {
      this.notificationLevel   = level;
      this.notificationMessage = message;
    };
  }
};
</script>

<style lang="scss" scoped>
$sidebar-color: #fafafa;
$sidebar-link-color1: #fafafa;
$sidebar-link-color2: #586575;
$sidebar-color1: #586575;
$sidebar-color2: #4a90aa;
$sidebar-color3: #586575;
$sidebar-color4: #E7DACB;

.dashboard {
  height: 100%;
  width: 100%;
}

.container {
  display: flex;
  height:100%;
}

nav {
  width: 200px;
  background: $sidebar-color1;
  height: 100%;
  color: $sidebar-color;

  .title {
    height: 35px;
    border-top: 5px solid $sidebar-color2;
    text-align: center;
    
    h1 {
      font-size: 1em;
      font-weight: 700;
      margin: 0;
      padding-top: 9px;
    }
  }

  .user {
    margin-top: 3px;
    padding: 2px 0;
    background: $sidebar-color3;
    text-align: center;

    img {
      border: 1px solid #eeeeee;
      background: #eeeeee;
      border-radius: 10px;
      position: relative;
      height: 16px;
      top: 2px;
    }

    span {
      margin-left: 3px;
      position: relative;
      top: -2px;
      font-size: 14px;
      font-weight: 600;
    }
  }

  .links {
    list-style-type: none;
    margin-top: 20px;
    padding: 0;

    a {
      color: $sidebar-link-color1;
      text-decoration: none;

      li {
        padding: 10px 20px;

        &:hover {
          text-decoration: underline;
        }
      }

      .fa {
        font-size: 14px;
        margin-right: 5px;
        &.red  { color: #ff6b77; }
        &.blue   { color: #7194e8; }
        &.green  { color: #47d268; }
        &.yellow { color: #f3d747; }
      }
     
      span {
        font-size: 14px;
        font-weight: 600;
      }
    }
  }

  .sublinks {
    background: $sidebar-color4;
    padding-top: 10px;
    padding-bottom: 5px;
    margin-bottom: 30px;

    .fa-caret-up {
      display: block;
      height: 0;
      position: relative;
      font-size: 25px;
      color: $sidebar-color4;
      top: -26px;
      left: 22px;
    }

    & > p {
      margin: 0;
      color: $sidebar-link-color2;
      font-size: 0.9rem;
      font-weight: 700;
      padding-left: 15px;
      padding-bottom: 15px;
    }
    
    & > ul {
      list-style-type: none;
      padding: 0;
      padding-left: 20px;

      li {
        color: $sidebar-link-color2;
        padding: 0;
        margin: 0px 0px 14px 10px;
      }
    }
  }
}

.content {
  width: 100%;
  overflow: scroll;

  header {
    height: 40px;
    border-bottom: 1px solid #efefef;
    display: flex;
    justify-content: space-between;

    .leftside {
      .title {
        color: #5a5a5a;
        margin-left: 10px;
        position: relative;
        top: 10px;
        font-size: 16px;
        font-weight: bold;
      }
    }

    .center {
      text-align: center;
      margin: 0;
      padding: 0;

      .title {
        position: relative;
        top: 7px;
        font-size: 18px;
        font-weight: bold;
      }
    }

    .rightside {
      position: relative;
      right: 20px;
      top: 7px;
    }
  }

  main {
    padding: 30px 40px;
  }
}
</style>
