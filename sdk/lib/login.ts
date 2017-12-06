import { api } from "./api";

export function login(url: string, username: string, password: string) {
    return api.login(url, username, password);
}
