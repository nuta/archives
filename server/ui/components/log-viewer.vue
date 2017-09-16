<template>
<table class="log">
  <thead>
    <td class="date">Time</td>
    <td class="device-name">Device</td>
    <td class="body">Body</td>
  </thead>
  <tbody>
    <tr v-for="line in lines">
      <td class="date">{{ line.time | strftime }}</td>
      <td class="device-name">{{ line.device_name }}</td>
      <td class="body">{{ line.body }}</td>
    </tr>
  </tbody>
</table>
</template>

<script>
import {strftime} from "js/filters";

export default {
  props: ["lines"],
  filters: { strftime }
};
</script>

<style lang="scss" scoped>
.log {
  width: 100%;
  font-family: monospace;
  font-size: 13px;
  line-height: 12px;
    border-collapse: collapse;

  thead {
    background: #fafafa;
    text-align: center;

    .date {
      width: 15%;
    }

    .device-name {
      width: 20%;
    }

    .body {
      width: 65%;
    }
  }

  td {
    border: 1px solid #efefef;

    &.date {
      text-align: center;
    }

    &.device-name {
      text-align: center;
    }
  }
}
</style>