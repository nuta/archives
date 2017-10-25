<template>
<dashboard-layout :title="deviceName">
  <div class="separated-sections">
    <form @submit.prevent="update">
      <section>
        <div class="left-column">
          <p class="title">Associated app</p>
          <p class="description"></p>
          </p>
        </div>
        <div class="right-column">
          <select v-model="associatedTo">
            <template v-for="app in apps">
              <option :value="app.name">{{ app.name }}</option>
            </template>
          </select>
        </div>
      </section>

      <section>
        <div class="left-column">
          <p class="title">Tag</p>
          <p class="description"></p>
          </p>
        </div>
        <div class="right-column">
          <input type="text" v-model="tag" placeholder="e.g. staging, group1, palo_alto">
        </div>
      </section>

      <input type="submit" value="Save">
    </form>
  </div>
</dashboard-layout>
</template>

<script>
import api from "js/api";
import DashboardLayout from "layouts/dashboard";

export default {
  components: { DashboardLayout },
  data() {
    return {
      deviceName: app.$router.currentRoute.params.deviceName,
      apps: [],
      associatedTo: '',
      tag: ''
    }
  },
  methods: {
    update() {
      api.updateDevice(this.deviceName, {
        app: this.associatedTo,
        tag: this.tag
      })
    }
  },
  async beforeMount() {
    const device = (await api.getDevice(this.deviceName)).json
    this.apps = (await api.getApps()).json
    this.associatedTo = device.app || this.apps[0].name
  }
};
</script>

<style lang="scss" scoped>
.separated-sections {
  section {
    display: flex;
    justify-content: space-between;

    &:not(:first-child) {
      margin-top: 30px;
      border-top: 1px solid #dadada;
    }

    .left-column {
      .title {
        font-size: 25px;
        font-weight: 600;
      }

      .description {
        font-size: 13px;
        color: #666;
      }
    }

    .right-column {
      align-self: center;
    }
  }
}
</style>
