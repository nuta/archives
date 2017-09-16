<template>
<div class="notifications">
  <template v-for="notification in notifications">
    <div class="notification" :class="notification.level">
      <i class="fa" :class="notification.level | icon" aria-hidden="true"></i>
      <span>{{ notification.message }}</span>
    </div>
  </template>
</div>
</template>

<script>
export default {
  props: ["level", "message"],
  data() {
    return {
      notifications: [],
      timer: undefined
    };
  },
  filters: {
    icon(level) {
      switch (level) {
        case "info": return "fa-info-circle";
        case "error": return "fa-times-circle";
        case "success": return "fa-check-circle";
      }
    }
  },
  watch: {
    message() {
      clearTimeout(this.timer);

      this.notifications.push({
        level: this.level,
        message: this.message
      });

      this.timer = setTimeout(() => {
        this.notifications = [];
      }, 3000);
    }
  }
}
</script>

<style lang="scss" scoped>
.notifications {
  position: absolute;
  bottom: 7px;
  right: 7px;

  .notification {
    margin-bottom: 4px;
    min-width: 200px;
    font-size: 13px;
    vertical-align: center;
    box-shadow: 1px 2px 2px rgba(0, 0, 0, 0.1);
    padding: 10px;
    border-radius: 3px;
    z-index: 3;

    &.error {
      color: #fefefe;
      background: #ee5577;
    }
  }
}
</style>
