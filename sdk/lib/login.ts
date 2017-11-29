import { api } from './api';

export function login(url, username, password) {
  return api.login(url, username, password)
}
