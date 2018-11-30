<template>
  <div class="notifications">
    <div class="notification" :class="[notification.type, notification.hidden]"
     v-for="(notification, index) in notifications" :key="index">
      {{ notification.title }}
    </div>
  </div>
</template>

<script>
import { setInterval, clearInterval } from 'timers';

export default {
  props: ['notifications']
}
</script>

<style lang="scss" scoped>
$notification-width: 500px;

.notifications {
  z-index: 2000;
  position: absolute;
  top: 0;
  left: 50%;
  margin-top: 10px;

  .notification {
    padding: 15px 20px;
    width: $notification-width;
    margin-left: -($notification-width / 2);
    color: var(--fg1-color);
    background-color: var(--bg1-color);
    font-weight: 600;
    font-size: 19px;

    &.danger {
      color: var(--negative-color);
    }

    &:not(:first-child) {
      margin-top: 10px;
    }
  }
}
</style>

