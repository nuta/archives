import VueRouter from 'vue-router'
import WizardView from 'views/wizard'

let routes = [
  { name: 'wizard', path: '/', component: WizardView },
  { path: '*', redirect: '/' }
]

export default new VueRouter({
  mode: "history",
  routes
})
