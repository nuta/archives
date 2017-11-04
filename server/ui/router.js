import VueRouter from 'vue-router'
import CreateAccountView from './views/create-account'
import ResetPasswordView from './views/reset-password'
import LoginView from './views/login'
import ForceLoginView from './views/force-login'
import SettingsView from './views/settings'
import AppsView from './views/apps'
import NewAppView from './views/apps/new'
import AppLogView from './views/apps/log'
import AppStoresView from './views/apps/stores'
import AppSettingsView from './views/apps/settings'
import CodeView from './views/apps/code'
import IntegrationsView from './views/apps/integrations'
import NewIntegrationView from './views/apps/new_integration'
import DeploymentsView from './views/apps/deployments'
import DevicesView from './views/devices'
import DeviceStoresView from './views/devices/stores'
import DeviceLogView from './views/devices/log'
import DeviceSettingsView from './views/devices/settings'

let routes = [
  { name: 'create-account', path: '/create-account', component: CreateAccountView },
  { name: 'reset-password', path: '/reset-password', component: ResetPasswordView },
  { name: 'login', path: '/login', component: LoginView },
  { name: 'force-login', path: '/force-login', component: ForceLoginView },
  { name: 'settings', path: '/settings', component: SettingsView },
  { name: 'home', path: '/apps', component: AppsView },
  { name: 'apps', path: '/apps', component: AppsView },
  { name: 'newApp', path: '/apps/new', component: NewAppView },
  { name: 'code', path: '/apps/:appName/code', component: CodeView },
  { name: 'deployments', path: '/apps/:appName/deployments', component: DeploymentsView },
  { name: 'appLog', path: '/apps/:appName/log', component: AppLogView },
  { name: 'integrations', path: '/apps/:appName/integrations', component: IntegrationsView },
  { name: 'newIntegration', path: '/apps/:appName/integrations/new', component: NewIntegrationView },
  { name: 'appStores', path: '/apps/:appName/stores', component: AppStoresView },
  { name: 'appSettings', path: '/apps/:appName/settings', component: AppSettingsView },
  { name: 'devices', path: '/devices', component: DevicesView },
  { name: 'deviceStores', path: '/devices/:deviceName/stores', component: DeviceStoresView },
  { name: 'deviceLog', path: '/devices/:deviceName/log', component: DeviceLogView },
  { name: 'deviceSettings', path: '/devices/:deviceName/settings', component: DeviceSettingsView }
]

export default new VueRouter({
  mode: 'history',
  routes
})
