import Vue from 'vue'
import VueRouter from 'vue-router'
import VueProgressBar from 'vue-progressbar'
import router from './router'
import App from './app'

Vue.use(VueRouter)
Vue.use(VueProgressBar, {
  color: 'rgb(143, 195, 255)',
  failedColor: 'red',
  height: '2px'
})

window.app = new Vue(Vue.util.extend({ router }, App))
app.$mount('#app')
