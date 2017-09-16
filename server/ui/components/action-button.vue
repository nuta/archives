<template>
  <button v-on:click="click" :class="{ positive: isPositive }" class="action-button">
    <i class="fa" :class="currentIcon" aria-hidden="true"></i>
    <span>{{ message }}</span>
  </button>
</template>

<script>
export default {
  props: ["icon", "state", "waitingMessage", "doingMessage", "doneMessage", "positive"],
  computed: {
    isPositive() {
      return this.positive !== undefined;
    },
    currentIcon() {
      switch (this.state) {
        case "waiting": return this.icon;
        case "doing":   return "fa-spinner fa-spin";
        case "done":    return "fa-check";
      }
    },
    message() {
      switch (this.state) {
        case "waiting": return this.waitingMessage;
        case "doing":   return this.doingMessage;
        case "done":    return this.doneMessage;
      }
    }
  },
  methods: {
    click: function() {
      this.$emit("click");
    }
  }
}
</script>

<style lang="scss" scoped>
.action-button {
  border-radius: 5px;
  justify-content: space-between;

  &.positive {
    color: #ececec;
    background: #47a068;
  }

  span {
    font-size: 15px;
    font-weight: 700;
  }
}
</style>
