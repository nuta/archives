<template>
  <div class="card" :class="{ clickable }" @click="onClick">
    <h4>{{ title }}</h4>
    <p class="subtitle">{{ subtitle }}</p>
    <div class="content">
      <slot></slot>
    </div>
  </div>
</template>

<script>
export default {
  props: ['title', 'subtitle', 'clickable'],
  methods: {
    onClick() {
      this.$emit('click')
    }
  }
}
</script>

<style lang="scss" scoped>
.card {
  border-radius: 4px;
  box-shadow: 0px 1px 0px rgba(0, 0, 0, .20);
  padding: 20px 15px;
  background: var(--bg0-color);
  border: 1px solid var(--border-color);

  &.clickable {
    user-select: none;

    &:hover {
      cursor: pointer;
    }
  }

  &:not(:first-child) {
    margin-top: 15px;
  }

  h4 {
    margin: 0;
    padding: 0;
    font-size: 20px;
  }

  .subtitle {
    color: var(--fg2-color);
  }

  .content {
    margin-top: 20px;
  }
}
</style>

