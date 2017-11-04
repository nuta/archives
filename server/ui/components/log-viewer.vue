<template>
<div class="log-viewer">
  <div class="tail-keeper-button-container">
    <button @click="toggleTailKeeper" :class="[keepTail ? 'uk-button-primary' : 'uk-button-secondary', 'uk-button', 'uk-button-small']">
      <span :uk-icon="[keepTail ? 'icon: play' : 'icon: arrow-down']"></span>
      Scroll to end of log
    </button>
  </div>

  <div class="log" v-show="lines.length > 0">
    <table>
      <tbody>
        <tr v-for="line in lines">
          <td class="time">{{ line.time | strftime }}</td>
          <td class="device">{{ line.device_name }}</td>
          <td class="type"><span :class="['uk-label', getTypeClass(line.body)]">{{ getTypeName(line.body) }}</span></td>
          <td class="body">{{ line.body }}</td>
        </tr>
      </tbody>
    </table>
  </div>

  <div class="bottom">
    <i class="fa fa-refresh fa-spin fa-1x fa-fw"></i>
    <span>Polling new log...</span>
    <span class="last-fetched">(fetched {{ lastFetched }} seconds ago)</span>
  </div>
</div>
</template>

<script>
import {strftime} from "js/filters";

export default {
  props: ["lines"],
  filters: { strftime },
  data() {
    return {
      keepTail: false,
      updatedAt: new Date(),
      lastFetched: 0
    }
  },
  watch: {
    lines() {
      if (this.keepTail) {
        this.$nextTick(() => {
          this.scrollToBottom()
        })
      }

      this.updatedAt = new Date()
    }
  },
  methods: {
    getTypeClass(line) {
      if (line.startsWith('@')) {
        return 'uk-label-success'
        } else if (line.startsWith('!')) {
        return 'uk-label-danger'
      } else {
        return 'uk-label'
      }
    },
    getTypeName(line) {
      if (line.startsWith('@')) {
        return 'EVENT'
        } else if (line.startsWith('!')) {
        return 'ERROR'
      } else {
        return 'INFO'
      }
    },
    scrollToBottom() {
      window.scrollTo(0, document.body.scrollHeight)
    },
    toggleTailKeeper() {
      this.keepTail = !this.keepTail
      if (this.keepTail) {
        this.scrollToBottom()
      }
    },
    updateLastFetched() {
      this.lastFetched =  Math.floor((new Date() - this.updatedAt) / 1000)
    }
  },
  beforeMount(){
    this.updateLastFetched()
    setInterval(() => {
      this.updateLastFetched()
    }, 1000)
  }
}
</script>

<style lang="scss" scoped>
.tail-keeper-button-container {
  position: fixed;
  bottom: 20px;
  right: 25px;
}

.log {
  padding: 10px 10px 5px 10px;
  background: #f4f4f4;
  border: 1px solid #eaeaea;

  table {
    width: 100%;
    font-family: 'Roboto Mono', monospace;
    font-size: 13px;
    line-height: 12px;
    border-collapse: collapse;

    tr:hover {
      background: #fff2a8;
    }

    .time {
      font-weight: 700;
      width: 10%;
    }

    .device {
      font-weight: 700;
      width: 10%;
    }

    .type {
      width: 5%;
    }

    .body {
      padding-left: 30px;
      width: 75%;
    }

    .uk-label {
      font-size: 10px;
      padding: 1px 5px;
    }
  }
}

.bottom {
  font-weight: 600;
  font-size: 15px;
  background: #e0e0e0;
  text-align: center;
  margin-bottom: 20px;
  padding: 10px;

  .last-fetched {
    margin-left: 5px;
    font-size: 12px;
  }
}
</style>
