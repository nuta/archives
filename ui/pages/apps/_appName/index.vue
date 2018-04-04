<template>
  <dashboard-layout title="Code" :appName="appName" no-padding="true">
    <editor :code="code" @changed="codeChanged"></editor>
    <footer>
      <div class="bottom-bar">
        <template v-if="this.devices.length === 0">
          <div>
            <nuxt-link :to="{ name: 'apps-appName-setup-device', params: { name: this.appName }}">
              <button class="primary">
                <i class="fa fa-magic" aria-hidden="true"></i>
                Setup a device before deploying
              </button>
            </nuxt-link>
            <button @click="deploy">
              <i class="fa fa-deploy" aria-hidden="true"></i>
              Deploy Anyway
            </button>
          </div>
        </template>
        <template v-else>
          <button @click="deploy" class="primary">
            <i class="fa fa-rocket" aria-hidden="true"></i>
            {{ deployButton }}
          </button>
        </template>

        <p class="caption">{{ caption }}</p>
      </div>
      <log-panel :appName="appName"></log-panel>
    </footer>
  </dashboard-layout>
</template>

<script>
import api from "~/assets/js/api"
import { buildApp } from "~/assets/js/build"
import { setLastUsedApp } from "~/assets/js/preferences"
import Editor from "~/components/editor"
import LogPanel from "~/components/log-panel"
import DashboardLayout from "~/components/dashboard-layout"

export default {
  components: { DashboardLayout, Editor, LogPanel },
  head: {
    link: [
      { rel: 'stylesheet', href: 'https://fonts.googleapis.com/css?family=Source+Code+Pro:400,600' },
    ],
    script: [
      { src: 'https://unpkg.com/babel-standalone@6/babel.min.js' }
    ]
  },
  data() {
    return {
      appName: this.$route.params.appName,
      code: '',
      devices: [],
      caption: 'Code will be automatically saved.',
      autosaveAfter: 3000,
      deployButton: "Deploy"
    }
  },
  methods: {
    async deploy() {
      this.deployButton = "Building..."
      const { image, debug } = await buildApp(this.code)

      this.deployButton = "Deploying...";
      const comment = "Deployment at " + (new Date()).toString();
      const r = await api.deploy(this.appName, image, debug, comment, null)

      this.deployButton = "Deployed";
      setTimeout(() => {
        this.deployButton = "Deploy";
      }, 3000)
    },
    codeChanged(newCode) {
      this.caption = 'Code will be saved when you stop typing...'
      if (this.autosaveTimer) {
        clearTimeout(this.autosaveTimer)
      }

      this.autosaveTimer = setTimeout(async () => {
        if (this.code === newCode) {
          this.caption = 'Code will be automatically saved.'
        } else {
          this.caption = 'Saving...'
          await api.saveFile(this.appName, 'app.js', newCode)
          this.caption = 'Saved'
          this.code = newCode
        }
      }, this.autosaveAfter)
    }
  },

  async mounted() {
    for (const file of await api.getFiles(this.appName)) {
      if (file.path === 'app.js') {
        this.code = file.body
      }
    }

    this.app = await api.getApp(this.appName)
    this.devices = await api.getAppDevices(this.appName)
    setLastUsedApp(this.appName)
  }
};
</script>

<style lang="scss" scoped>
.dashboard-layout {
  background: var(--bg0-color) !important;
  height: 100vh !important;
}

footer {
  .bottom-bar {
    padding: 5px 10px;
    display: flex;
    justify-content: space-between;
    background: var(--bg0-color);

    .caption {
      color: var(--caption-color);
      font-size: 14px;
      display: inline;
    }
  }
}
</style>
