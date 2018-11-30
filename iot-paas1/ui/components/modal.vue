<template>
  <div class="modal-container" v-if="active">
    <div class="modal">
      <header>
        <h3>{{ title }}</h3>
        <span class="close-button" @click="close">
          <i class="far fa-times-circle"></i>
        </span>
      </header>
      <slot class="main"></slot>
    </div>
    <div class="modal-wallpaper"></div>
  </div>
</template>

<script>
export default {
  props: {
    title: String,
  },
  data() {
    return {
      active: false
    }
  },
  methods: {
    open() {
      this.active = true
    },
    close() {
      this.active = false
    }
  }
}
</script>

<style lang="scss" scoped>
.modal-wallpaper {
  position: absolute;
  top: 0;
  left: 0;
  height: 100vh;
  width: 100vw;
  background: rgba(0, 0, 0, 0.6);
  z-index: 1;
}

.modal {
  position: absolute;
  z-index: 2;
  padding: 30px;
  width: 500px;
  margin: 30px 50%;
  transform: translateX(-50%);
  background: #ffffff;
  border: 1px solid #e1e1e1;
  border-radius: 4px 4px;
  box-shadow: 0px 1px 0px rgba(0, 0, 0, .20);

  header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    margin-bottom: 35px;

    h3 {
      margin: 0;
      padding: 0;
    }
  }

  .close-button {
    color: #9c9c9c;
    font-size: 30px;

    &:hover {
      cursor: pointer;
      color: #878787;
    }
  }
}
</style>
