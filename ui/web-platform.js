export function loadCredentials() {
  return JSON.parse(localStorage.getItem('credentials'))
}

export function saveCredentials(credentials) {
   localStorage.setItem('credentials', JSON.stringify(credentials))
}

export function removeCredentials() {
  localStorage.removeItem('credentials')
}

export function getServerUrl() {
  return ''
}

export function appendToLog() {
  // Do nothing.
}
