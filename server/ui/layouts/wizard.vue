<template>
  <div class="wizard">
    <div class="uk-card uk-card-default uk-card-body uk-width-1-2@m">
      <header>
        <h1 class="uk-card-title">{{ title }}</h1>
      </header>
      <main>
        <slot></slot>
      </main>
    </div>
  </div>
</template>

<script>
export default {
  props: [ 'title' ]
}
</script>

<style lang="scss" scoped>
.wizard {
  height: 100vh;
  background: linear-gradient(to bottom, #7474bf, #348ac7) !important;
  padding-top: 30px;
  box-sizing: border-box;
}

.uk-card {
  width: 500px;
  margin: 50px auto 0;
  padding: 20px;
  border-radius: 5px;
  background: #ffffff;
}
</style>
