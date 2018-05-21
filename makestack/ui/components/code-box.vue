<template>
  <div class="code-box">
    <slot></slot>
  </div>
</template>

<style lang="scss" scoped>
.code-box {
  margin: 20px 50px;
  padding: 5px;
  white-space: pre;
  line-height: 1.4rem;
  text-align: left;
  max-height: 500px;
  overflow: scroll;
  font-family: 'Source Code Pro', monospace;
  background: var(--bg2-color);
}
</style>
