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
$ui-fonts: "Open Sans", sans-serif;

html, body {
  height:100%;
}

body {
  margin: 0;
  font-size: 15px;
  font-family: $ui-fonts;
  color: $default-font-color;
}

</style>
