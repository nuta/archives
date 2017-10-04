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
      deviceName: app.$router.currentRoute.params.deviceName,
      log: []
    };
  },
  beforeMount() {
    let since = Date.now();
    api.getDeviceLog(this.deviceName).then(r => {
      this.log = r.json.lines;
      setInterval(() => {
        api.getDeviceLog(this.deviceName, since).then(r => {
          since = Date.now();
          this.log.concat(r.json.lines);
        }).catch(error => notify("error", error));
      }, 3000);
    }).catch(error => notify("error", error));
  }
};
</script>