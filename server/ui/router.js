import VueRouter from 'vue-router'
import WelcomeView from './views/welcome'
import CreateAccountView from './views/create-account'
import ResetPasswordView from './views/reset-password'
import LoginView from './views/login'
import ForceLoginView from './views/force-login'
import SettingsView from './views/settings'
import AppsView from './views/apps'
import NewAppView from './views/apps/new'
import AppLogView from './views/apps/log'
import AppConfigsView from './views/apps/configs'
import AppSettingsView from './views/apps/settings'
import CodeView from './views/apps/code'
import IntegrationsView from './views/apps/integrations'
import NewIntegrationView from './views/apps/new_integration'
import DeploymentsView from './views/apps/deployments'
import DevicesView from './views/devices'
import DeviceConfigsView from './views/devices/configs'
import DeviceLogView from './views/devices/log'
import DeviceSettingsView from './views/devices/settings'

let routes = [
  { name: 'welcome', path: '/', component: WelcomeView },
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
  { name: 'appConfigs', path: '/apps/:appName/configs', component: AppConfigsView },
  { name: 'appSettings', path: '/apps/:appName/settings', component: AppSettingsView },
  { name: 'devices', path: '/devices', component: DevicesView },
  { name: 'deviceConfigs', path: '/devices/:deviceName/configs', component: DeviceConfigsView },
  { name: 'deviceLog', path: '/devices/:deviceName/log', component: DeviceLogView },
  { name: 'deviceSettings', path: '/devices/:deviceName/settings', component: DeviceSettingsView }
]

export default new VueRouter({
  mode: ROUTER_MODE,
  routes
})
