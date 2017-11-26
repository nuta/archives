<template>
  <div id="app">
    <router-view></router-view>
  </div>
</template>

<script>
export default {
};
</script>

<style lang="scss">
$default-font-color: #183f18;

html, body {
  height: 100%;
}

body {
  margin: 0;
  font-size: 15px;
  font-family: -apple-system, BlinkMacSystemFont, Ubuntu, Cantarell, sans-serif;
  color: $default-font-color;
  -webkit-app-region: drag;
}

#app {
}
</style>
