<template>
  <simple-layout>
    <header>
      <h1>Create a new app</h1>
      <nuxt-link :to="{ name: 'apps' }" class="return-link">
        <i class="fas fa-chevron-left"></i>
        Return to app
      </nuxt-link>
    </header>
    <main>
      <form @submit.prevent="createApp">
        <div class="field">
          <label>Name</label>
          <input type="text" v-model="name" placeholder="Name" required="required" autofocus>
        </div>

        <div class="action">
          <input type="submit" value="Create" class="primary">
        </div>
      </form>
    </main>
  </simple-layout>
</template>

<script>
import api from "~/assets/js/api";
import SimpleLayout from "~/components/simple-layout";

export default {
  components: { SimpleLayout },
  data: () => {
    return {
      name: ""
    };
  },
  methods: {
    async createApp() {
      await api.createApp(this.name, 'nodejs')
      this.$router.push({ name: 'apps-appName', params: { name: this.name } })
    }
  }
}
</script>
