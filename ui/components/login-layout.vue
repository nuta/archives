<template>
  <div class="login-layout">
    <header>
      <h1>MakeStack</h1>
      <p>A "batteries-included" IoT PaaS for super-rapid prototyping.</p>
    </header>
    <main>
      <slot></slot>
    </main>
  </div>
</template>

<style lang="scss" scoped>
.login-layout {
  max-width: 500px;
  margin: 30px auto;

  header {
    text-align: center;
    border-bottom: 2px solid #eaeaea;
    padding-bottom: 25px;

    h1 {
      font-family: "Roboto", sans-serif;
      font-weight: 900;
      font-size: 75px;
      margin: 0;
    }

    p {
      font-family: "Lato", sans-serif;
      font-weight: 300;
      font-style: italic;
      margin: 0;
    }
  }

  main {
    margin-top: 50px;
  }
}
</style>
