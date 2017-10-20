<template>
<dashboard-layout title="Create a new app">
  <form v-on:submit.prevent="createApp">
    <fieldset>
      <label>Name</label>
      <input type="text" v-model="appName" required="required" autofocus placeholder="Name">
    </fieldset>

    <fieldset>
      <label>API</label>
      <div class="columns">
        <radiobox name="api" value="linux" title="A full LInux Environment" description="A feature-rich Linux Environment featuring Node.js with Arduino-like API." selected></radiobox>
      </div>
    </fieldset>
    <input type="submit" value="Create">
  </form>
</dashboard-layout>
</template>

<script>
import api from "js/api";
import Radiobox from "components/radiobox";
import DashboardLayout from "layouts/dashboard";

export default {
  components: { DashboardLayout, Radiobox },
  data() {
    return {
      appName: ""
    };
  },
  methods: {
    createApp() {
      let appApi = document.querySelector("input[name=api]:checked").value;
      api.createApp(this.appName, appApi).then(() => {
        this.$router.push({ name: "apps" });
      }).catch(reason => {
        debugger;
      });
    }
  }
};
</script>
