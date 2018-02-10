<template>
  <div class="dropdown">
    <div class="dropdown-drawer" ref="drawer" @click="drawerClicked">
      <span :style="titleStyle">
        {{ selected }}
        <i class="fas" :class="[caretType]"></i>
      </span>
    </div>

    <ul class="dropdown-items" ref="items" :class="{ hide }">
      <li v-for="(item, index) in items" :class="{ divider: item.divider }" @click="itemClicked(item)" :key="index">
        <template v-if="item.to">
          <nuxt-link :to="item.to" class="link-item">
            <i class="fas" :class="['fa-' + item.icon]" v-if="item.icon"></i>
            <span :class="{ bold: item.bold }">{{ item.title }}</span>
          </nuxt-link>
        </template>
        <template v-else>
          <span>
            <i class="fas" :class="['fa-' + item.icon]" v-if="item.icon"></i>
            <span :class="{ bold: item.bold }">{{ item.title }}</span>
          </span>
        </template>
      </li>
    </ul>
  </div>
</template>

<script>
export default {
  props: ['items', 'selected', 'title-style'],
  data() {
    return {
      hide: true,
      rightAligned: false
    }
  },
  computed: {
    caretType() {
      return this.hide ? 'fa-caret-down' : 'fa-caret-up';
    },
  },
  methods: {
    drawerClicked() {
      this.hide = !this.hide;
    },
    itemClicked(item) {
      this.$emit('select', item)
    }
  },
  mounted() {
    const drawer = this.$refs.drawer
    const items = this.$refs.items
    if (drawer.offsetLeft + items.clientWidth + 50 > window.innerWidth) {
      const offset = items.clientWidth - drawer.clientWidth
      items.style.marginLeft = `-${offset}px`
    }
  }
}
</script>

<style lang="scss" scoped>
.dropdown {
  .dropdown-drawer {
    padding: 5px;
    width: fit-content;
    user-select: none;
    border: 1px solid transparent;

    &:hover {
      cursor: pointer;
      border: 1px solid #e1e1e1;
    }

    .fas {
      margin-top: -3px;
      padding-left: 5px;
      font-size: 15px;
      vertical-align: middle;
      color: #afafaf;
    }
  }

  .dropdown-items {
    position: absolute;
    z-index: 1000;
    margin-top: 5px;
    list-style-type: none;
    border: 1px solid var(--border-color);
    border-radius: 5px;
    background: var(--bg0-color);
    padding: 8px 5px;
    min-width: 150px;

    li {
      padding: 5px 10px;
      text-align: left;
      user-select: none;

      &:hover {
        cursor: pointer;
        background: var(--bg1-color);
      }

      &.divider {
        border-top: 1px solid var(--border-color);
        padding-top: 10px;
        margin-top: 10px;
      }

      a {
        display: inline-block;
        width: 100%;
      }

      .bold {
        font-weight: 600;
      }

      .fas {
        padding-right: 5px;
      }

      .link-item {
        color: inherit;
        text-decoration: none;
      }
    }

    &.hide {
      visibility: hidden;
    }
  }
}
</style>
