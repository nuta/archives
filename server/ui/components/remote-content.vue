<template>
<div>
  <div v-if="loading" class="loading uk-placeholder uk-text-center">
    <i class="fa fa-spinner fa-spin fa-lg fa-fw"></i>
    <span>Loading</span>
  </div>

  <div v-show="!loading && empty" uk-alert class="welcome uk-alert-primary uk-width-xlarge uk-position-center">
    <h5>
      <i class="fa fa-birthday-cake" aria-hidden="true"></i>
      <slot name="welcome-title"></slot>
    </h5>
    <div>
      <slot name="welcome-description"></slot>
    </div>
  </div>

  <div v-show="!loading && !empty" class="content">
    <slot name="content" v-show="!loading && !empty" class="content"></slot>
  </div>
</div>
</template>

<script>
export default {
  props: [ 'loading', 'content' ],
  computed: {
    empty() {
      if (this.content instanceof Array) {
        return this.content.length === 0
      }

      return this.content === null
    }
  }
};
</script>

<style lang="scss" scoped>
.loading {
  margin-top: 30px;
  font-weight: 600;
}

.welcome {
  h5 > * {
    display: inline-block;
  }

  h5 > i {
    margin-right: 5px;
  }
}
</style>
