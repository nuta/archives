<template>
  <div class="card" :class="{ clickable: clickable || to }" @click="onClick">
    <h4>{{ title }}</h4>
    <p class="subtitle">{{ subtitle }}</p>
  </div>
</template>

<script>
export default {
  props: ['title', 'subtitle', 'clickable', 'to'],
  methods: {
    onClick() {
      if (this.to) {
        this.$router.push(this.to)
      } else {
        this.$emit('click')
      }
    }
  }
}
</script>

<style lang="scss" scoped>
.card {
  border-radius: 4px;
  box-shadow: 0px 1px 0px rgba(0, 0, 0, .20);
  padding: 20px 10px;
  background: var(--bg0-color);
  border: 1px solid var(--border-color);

  &.clickable {
    user-select: none;

    &:hover {
      cursor: pointer;
      box-shadow: 0px 0px 3px 3px var(--link-color);
    }
  }

  h4 {
    color: var(--fg0-color);
    margin: 0;
    padding: 0;
    font-size: 18px;
    margin-bottom: 10px;
  }

  .subtitle {
    color: var(--fg2-color);
    margin: 0;
    font-size: 15px;
  }

  .content {
    margin-top: 20px;
  }
}
</style>

