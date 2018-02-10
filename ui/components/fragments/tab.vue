<template>
  <section v-show="active">
    <slot></slot>
  </section>
</template>

<script>
export default {
  props: ['name'],
  data() {
    return {
      active: false
    }
  }
}
</script>
