<template>
<dashboard-layout title="Log">
  <log-viewer :lines="log"></log-viewer>
</dashboard-layout>
</template>

<script>
import api from "js/api";
import LogViewer from "components/log-viewer";
import DashboardLayout from "layouts/dashboard";

export default {
  components: { DashboardLayout, LogViewer },
  data() {
    return {
      appName: app.$router.currentRoute.params.appName,
      log: [],
      timer: undefined
    };
  },
  beforeMount() {
    let since = Date.now();
    api.getAppLog(this.appName).then(r => {
      this.log = r.json.lines;
      this.timer = setInterval(() => {
        api.getAppLog(this.appName, since)
          .then(r => {
            since = Date.now();
            this.log.concat(r.json.lines);
          }).catch(error => notify("error", error));
      }, 3000);
    }).catch(error => notify("error", error));
  },
  beforeRouteLeave (to, from, next) {
    clearInterval(this.timer);
    next();
  }
};
</script>