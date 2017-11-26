import VueRouter from 'vue-router'
import InstallerView from 'views/installer'
import LoginView from 'views/login'

let routes = [
  { name: 'installer', path: '/', component: InstallerView },
  { name: 'login', path: '/login', component: LoginView },
  { path: '*', redirect: '/' }
]

export default new VueRouter({
  mode: 'hash',
  routes
})
