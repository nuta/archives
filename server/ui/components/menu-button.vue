<template>
<div>
  <button class="menu-button" ref="button" v-on:click="toggle">
    <i :class="[icon]" class="fa" aria-hidden="true"></i>
    <span>{{ title }}</span>
  </button>

  <div class="menu-wrapper">
    <div class="menu" ref="menu">
      <ul>
        <template v-for="element in elements">
          <li @click="click(element.event)">{{ element.title }}</li>
        </template>
      </ul>
    </div>
  </div>
</div>
</template>

<script>
export default {
  props: ["icon", "title", "elements"],
  methods: {
    click(event) {
      this.toggle();
      this.$emit(event);
    },
    toggle() {
      if (this.$refs.menu.style.display == "block") {
        this.$refs.menu.style.display = "none";
      } else {
        this.$refs.menu.style.display = "block";
        let offset = this.$refs.menu.offsetWidth - this.$refs.button.offsetWidth;
        this.$refs.menu.style.right = `${offset}px`;
      }
    }
  }
};
</script>

<style lang="scss" scoped>
.menu-wrapper {
  width: 0;
  height: 0;

  .menu {
    display: none;
    position: relative;
    top: 10px;
    height: fit-content;
    width: fit-content;

    z-index: 1000;

    background: #ffffff;
    border: 1px solid #cccccc;
    border-radius: 5px;
    box-shadow: 2px 2px 2px rgba(0, 0, 0, 0.1);

    ul {
      list-style-type: none;
      margin: 10px;
      padding: 0;

      li {
        padding: 7px;
        white-space: nowrap;

        &:hover {
          background: #efefef;
          cursor: pointer;
        }
      }
    }
  }
}
</style>
