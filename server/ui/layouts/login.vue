<template>
  <div class="login">
    <main>
      <slot></slot>
    </main>
  </div>
</template>

<style lang="scss" scoped>
.login {
  height: 100%;
  background: #fafafa;
  padding-top: 30px;
}

main {
  width: 500px;
  margin: 50px auto 0;
  padding: 20px;
  background: #ffffff;
  border-radius: 5px;
}
</style>
