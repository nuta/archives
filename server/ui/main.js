import Vue from 'vue'
import VueRouter from 'vue-router'
import VueHead from 'vue-head'
import VueProgressBar from 'vue-progressbar'
import router from './router'
import App from './app'

Vue.use(VueRouter)
Vue.use(VueHead)
Vue.use(VueProgressBar, {
  color: '#ffba00',
  failedColor: 'red',
  height: '5px'
})

Vue.prototype.$Notification = {
  success(message) {
    UIkit.notification(message, { status: 'success', pos: 'bottom-center' })
  },
  error(message) {
    UIkit.notification(message, { status: 'danger', pos: 'bottom-center' })
  }
}

window.app = new Vue(Vue.util.extend({ router }, App))
app.$mount('#app')
