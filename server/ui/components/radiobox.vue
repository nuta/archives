<template>
<div class="radiobox" v-on:click="select" ref="box">
  <input type="radio" :name="name" :value="value" ref="radio">
  <div class="body">
     <div class="title">{{ title }}</div>
     <div class="description">{{ description }}</div>
  </div>
</div> 
</template>

<script>
export default {
  props: ["name", "value", "title", "description", "selected"],
  methods: {
    select: function() {
      document.querySelectorAll(`input[name=${this.name}]`).forEach(el => {
        el.checked  = el == this.$refs.radio;
        el.parentElement.classList = (el == this.$refs.radio)? "radiobox selected" : "radiobox";
      });
    }
  },
  mounted: function() {
    if (this.selected != undefined) {
      this.$refs.radio.checked = true;
      this.$refs.radio.parentElement.classList = "radiobox selected";
    }
  }
};
</script>

<style lang="scss" scoped>
.radiobox {
  background: #fafafa;
  padding: 20px 20px 20px 13px;
  border: 1px #eeeeee solid;
  display: flex;
  width: 100%;

  &.selected {
    background: #c0e0ff;
    border: 1px #7788aa solid;
  }

  &:hover {
    cursor: pointer;
  }

  input[type=radio] {
    display: inline-block;
    position: relative;
    width: 20px;
    top: 2px;
  }

  .body {
    margin-left: 5px;

    .title {
      font-weight: bold;
      display: inline;
    }

    .description {
      margin-top: 5px;
    }
  }
}
</style>
