import fs from 'fs'
import path from 'path'
import os from 'os'
import { ipcRenderer } from 'electron'

export function loadCredentials() {
  return ipcRenderer.sendSync('loadCredentials')
}

export function saveCredentials(credentials) {
  ipcRenderer.sendSync('saveCredentials', credentials)
}

export function removeCredentials() {
  ipcRenderer.sendSync('saveCredentials', {})
}

export function getServerUrl() {
  return ipcRenderer.sendSync('loadCredentials').url
}

export function appendToLog(message) {
  fs.appendFileSync(
    path.resolve(os.homedir(), '.makestack/desktop.log'),
    message
  )
}
