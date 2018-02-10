import Cookies from 'js-cookie'

export function getCurrentTheme() {
  return Cookies.get('theme') || 'simple'
}

export function setTheme(theme) {
  return Cookies.set('theme', theme)
}

export function getLastUsedApp() {
  return Cookies.get('last-used-app')
}

export function setLastUsedApp(appName) {
  return Cookies.set('last-used-app', appName)
}
