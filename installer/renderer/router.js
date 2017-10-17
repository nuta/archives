import VueRouter from 'vue-router'
import WizardView from 'views/wizard'
import LoginView from 'views/login'

let routes = [
  { name: 'wizard', path: '/', component: WizardView },
  { name: 'login', path: '/login', component: LoginView },
  { path: '*', redirect: '/' }
]

export default new VueRouter({
  mode: 'hash',
  routes
})
