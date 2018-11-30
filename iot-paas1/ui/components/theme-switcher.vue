<template>
  <div class="theme-switcher">
    <div v-for="theme in themes" :key="theme"
     :class="[ theme + '-theme', 'theme-button',  currentTheme == theme ? 'active' : '' ]"
     @click="saveTheme(theme)" :data-balloon="theme" data-balloon-pos="down"
     @mouseover="previewTheme(theme)" @mouseleave="previewTheme(savedTheme)"></div>
  </div>
</template>

<script>
import { getCurrentTheme, setTheme } from "~/assets/js/preferences";

export default {
  data() {
    return {
      currentTheme: getCurrentTheme(),
      savedTheme: getCurrentTheme(),
      themes: ['simple', 'monokai', 'solarized-light']
    }
  },
  methods: {
    previewTheme(theme) {
      document.body.classList.remove(this.theme + '-theme')
      document.body.classList.add(theme + '-theme')
      this.theme = theme
    },
    saveTheme(theme) {
      this.theme = theme
      this.savedTheme = theme
      setTheme(theme)
    },
  }
}
</script>

<style lang="scss" scoped>
@import "~assets/css/theme";

.theme-switcher {
  border-radius: 10px;
  padding: 20px 30px;
  width: 200px;
  margin: 0 auto;
  display: flex;
  justify-content: space-between;

  .theme-button {
    width: 30px;
    height: 30px;
    background: var(--bg0-color);
    border-radius: 50%;
    border: 1px solid var(--border-color);

    &:hover {
      cursor: pointer;
    }
  }
}
</style>
