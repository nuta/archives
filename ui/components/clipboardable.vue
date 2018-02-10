<template>
  <div class="clipboardable">
    <div v-if="textarea" class="textarea">{{ text }}</div>
    <div v-else class="oneline">
      <span ref="text">{{ text }}</span>
    </div>
  </div>
</template>

<script>
export default {
  props: ['text', 'textarea', 'hidden']
  // TODO: copy to clipboard
}
</script>

<style lang="scss" scoped>
.clipboardable {
  font-family: 'Source Code Pro', monospace;
  background: var(--bg1-color);
  border-radius: 7px;
  padding: 13px 10px;
  border: 1px solid var(--border-color);

  .textarea {
    white-space: pre;
    max-height: 300px;
    overflow: scroll;
  }

  .fa-clipboard {
    color: #5a5a5a;
    margin-left: 20px;

    &:hover {
      cursor: pointer;
    }
  }
}
</style>
