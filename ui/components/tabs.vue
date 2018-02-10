<template>
  <div class="tabs">
    <ul class="tab-items">
      <li v-for="item of items" :class="{ active: item === active }" @click="select(item)">
        {{ item }}
      </li>
    </ul>
    <div class="tab-contents">
      <div class="tab-content" ref="content">
        <slot></slot>
      </div>
    </div>
  </div>
</template>

<script>
export default {
  data() {
    return {
      items: [],
      contents: [],
      active: null
    }
  },
  methods: {
    select(item) {
      this.active = item
      for (const child of this.$children) {
        child.active = child.name === item
      }
    }
  },
  mounted() {
    this.items = this.$children.map(child => child.name)
    this.select(this.items[0])
  }
}
</script>

<style lang="scss" scoped>
.tabs {
  margin-top: 15px;

  .tab-items {
    margin: 0;
    padding: 0;
    list-style-type: none;

    li {
      padding: 10px;
      display: inline-block;
      font-weight: 600;
      color: var(--fg1-color);
      margin-bottom: -1px;
      border: 1px solid transparent;
      border-radius: 4px 4px 0px 0px;

      &.active {
        color: var(--fg0-color);
        background: var(--bg0-color);
      }

      &:hover {
        cursor: pointer;
      }
    }
  }

  .tab-contents {
      background: var(--bg0-color);
      border-radius: 0px 0px 4px 4px;
      box-shadow: 0px 1px 0px rgba(0, 0, 0, .20);
      padding: 30px;
  }
}
</style>

