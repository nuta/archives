<template>
  <div class="simple-layout">
    <main>
      <slot></slot>
    </main>
  </div>
</template>

<style lang="scss">
.simple-layout {
  color: var(--fg0-color);
  margin-bottom: 50px;

  & > main {
    max-width: 500px;
    padding-top: 30px;
    margin: 0px auto;

    & > header {
      text-align: center;
      border-bottom: 2px solid var(--border-color);
      padding-bottom: 25px;
      margin-bottom: 25px;

      h1 {
        font-family: "Roboto", sans-serif;
        font-weight: 900;
        font-size: 50px;
        margin: 0;
        margin-bottom: 10px;
      }

      p {
        font-family: "Lato", sans-serif;
        font-weight: 300;
        font-style: italic;
        font-size: 20px;
        margin: 0;
      }
    }
  }
}
</style>
