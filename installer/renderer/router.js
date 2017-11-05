import VueRouter from 'vue-router'
import InstallView from 'views/install'
import LoginView from 'views/login'

let routes = [
  { name: 'install', path: '/', component: InstallView },
  { name: 'login', path: '/login', component: LoginView },
  { path: '*', redirect: '/' }
]

export default new VueRouter({
  mode: 'hash',
  routes
})
