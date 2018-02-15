<template>
  <div class="log-panel">
    <div class="log-panel-drawer" @click="togglePanel">
      <i :class="[ 'fas', drawerIcon ]"></i>
      {{ drawerMsg }}
    </div>

    <div class="lines" v-show="showPanel" ref="lines">
      <div class="auto-scroll-button" :class="{ activated: autoScroll }" @click="autoScroll = !autoScroll">
        <i class="fas fa-angle-double-down"></i>
      </div>
      <p class="line" v-for="line in lines" :key="line.time" :class="[line.type]">
        <span class="time">[{{ line.time | date }}]</span>
        <span class="device">{{ line.device }}:</span>
        <span class="body">{{ line.body }}</span>
      </p>

      <p class="polling-message">
        <i class="fas fa-circle-notch fa-spin"></i>
        Polling new log... (fetched {{ lastFetched }} seconds ago)
      </p>
    </div>
  </div>
</template>

<script>
import api from "~/assets/js/api"
import { setInterval, clearInterval } from 'timers';

export default {
  props: ['appName'],
  data() {
    return {
      showPanel: false,
      lastFetchedAt: null,
      lastFetched: null,
      log: [],
      drawerMsg: 'Open Log',
      drawerIcon: 'fa-angle-double-up',
      autoScroll: true
    }
  },
  filters: {
    date(unixtime) {
      const date = new Date(unixtime * 1000)
      return `${date.getMonth()}/${date.getDate()} ${date.getHours()}:${date.getMinutes()}`
    }
  },
  computed: {
    lines() {
      const lines = []
      let prevType = 'normal'
      for (const line of this.log) {
        let type, body
        switch (line.body[0]) {
          case '@':
            type = 'event'
            body = line.body
            break
          case '!':
            type = 'error'
            body = line.body.slice(1)
            break
          case '\\':
            type = prevType
            body = line.body.slice(1)
            break
          default:
            type = 'normal'
            body = line.body
        }

        prevType = type
        lines.push({
          type,
          body,
          device: line.device,
          time: line.time
        })
      }

      if (this.autoScroll) {
        this.$nextTick(() => {
           this.$refs.lines.scrollTo(0, this.$refs.lines.scrollHeight)
        })
      }

      return lines
    }
  },
  methods: {
    togglePanel() {
      if (this.showPanel) {
        this.showPanel = false
        this.drawerIcon = 'fa-angle-double-up'
        this.drawerMsg = 'Show Log'
        this.disableLogFetch()
      } else {
        this.showPanel = true
        this.drawerIcon = 'fa-angle-double-down'
        this.drawerMsg = 'Hide Log'
        this.enableLogFetch()
      }
    },
    async fetchNewLines() {
      const lines = (await api.getAppLog(this.appName, this.lastFetchedAt)).lines || []
      this.lastFetchedAt = new Date()
      return lines
    },
    updateLastFetched() {
      this.lastFetched = Math.floor((new Date() - this.lastFetchedAt) / 1000)
    },
    async enableLogFetch() {
      this.log = await this.fetchNewLines()
      this.logFetchTimer = setInterval(async () => {
        this.log = this.log.concat(await this.fetchNewLines())
      }, 5000)

      this.updateLastFetched()
      this.updateLastFetchedTimer =  setInterval(() => {
        this.updateLastFetched()
      }, 1000)
    },
    disableLogFetch() {
      if (this.logFetchTimer) {
        clearInterval(this.logFetchTimer)
      }

      if (this.updateLastFetchedTimer) {
        clearInterval(this.updateLastFetchedTimer)
      }
    },
     beforeDestroy() {
       this.disableLogFetch()
     }
  }
}
</script>

<style lang="scss" scoped>
.log-panel {
  width: 100vw;
  background: var(--bg1-color);

  .log-panel-drawer {
    padding: 5px 0px;
    font-size: 15px;
    font-weight: 600;
    text-align: center;
    user-select: none;
    vertical-align: bottom;

    &:hover {
      cursor: pointer;
    }
  }

  .auto-scroll-button {
    position: absolute;
    width: 20px;
    font-size: 18px;
    text-align: center;
    right: 10px;
    padding: 10px;
    background: var(--border-color);
    border-radius: 20px;

    &:hover {
      cursor: pointer;
    }

    &.activated {
      background: var(--positive-color);
    }
  }

  .lines {
    height: 400px;
    overflow: scroll;
    font-family: 'Source Code Pro', monospace;
    padding: 0px 20px;
    padding-bottom: 10px;

    p {
      padding: 5px 0;
      margin: 0;

      &:hover {
        background: var(--bg0-color);
      }

      &.polling-message {
        padding-top: 15px;
        font-weight: 600;

         &:hover {
           background: inherit;
         }
      }

      &.event {
        color: var(--accent-color);
        font-weight: 600;
      }

      &.error {
        color: var(--negative-color);
        font-weight: 600;
      }

      .time {
        margin-right: 10px;
      }

      .device {
        margin-right: 5px;
        margin-right: 10px;
        font-weight: 600;
      }
    }
  }
}
</style>
