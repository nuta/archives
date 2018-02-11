<template>
  <dashboard-layout title="Setup a Device" :appName="appName">
    <guide-box>
      <h1>Installing Agent on your devices</h1>
      <p>You can run MakeStack Agent on your device that is running macOS and
        Linux distrbutions like Raspbian:</p>
      <code-box>
      $ npm install -g makestack-sdk
      $ makestack login
      $ makestack register {{ deviceName }}
      $ makestack add-device --app {{ appName }} {{ deviceName }}
      $ makestack run
      </code-box>
      <a href="https://www.npmjs.com/package/makestack-sdk">
        <button class="primary">Go to npmjs.com</button>
      </a>
    </guide-box>
  </dashboard-layout>
</template>

<script>
import api from "~/assets/js/api"
import DashboardLayout from "~/components/dashboard-layout"
import GuideBox from "~/components/guide-box"
import CodeBox from "~/components/code-box"

export default {
  components: { DashboardLayout, GuideBox, CodeBox },
  data() {
    return {
      appName: this.$route.params.appName,
      deviceName: 'DEVICE_NAME'
    }
  }
}
</script>
