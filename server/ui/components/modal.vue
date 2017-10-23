<template>
<div v-if="active">
  <div class="background"></div>
  <div class="modal" :class="animation">
    <div class="header">
      <span class="title">{{ title }}</span>
      <i @click="close" class="fa fa-window-close close" aria-hidden="true"></i>
    </div>
    <div class="content">
      <slot></slot>
    </div>
  </div>
</div>
</template>

<script>
import {strftime} from "js/filters";

export default {
  props: ["title", "active"],
  filters: { strftime },
  data() {
    return {
      animation: {
        animated: false,
        zoomIn: false
      }
    };
  },
  watch: {
    active(state) {
      this.animation.animated = false;
      this.animation.zoomIn   = state;
      this.animation.animated = true;
    }
  },
  methods: {
    close() {
      this.$emit('close');
    }
  }
};
</script>

<style lang="scss" scoped>
.background {
  position: fixed;
  top: 0;
  left: 0;
  height: 100vh;
  width: 100vw;
  z-index: 9;
  background: #000;
  opacity: 0.3;
}

.zoomIn {
  animation-duration: 0.3s;
}

.modal {
  position: fixed;
  width: 500px;
  top: 100px;
  left: 50%;
  margin-left: -250px;
  padding: 10px;
  z-index: 10;
  background: #ffffff;
  border: 1px solid #efefef;
  border-radius: 3px;

  .header {
    display: flex;
    justify-content: space-between;

    .title {
      font-weight: 600;
      font-size: 1.3rem;
    }

    .close {
      color: #777;
      font-size: 20px;

      &:hover {
        color: #555;
        cursor: pointer;
      }

      &:active {
        color: #da7a7a;
      }
    }
  }

  .content {
    margin-top: 30px;
  }
}
</style>
