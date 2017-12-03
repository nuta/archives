import * as util from "util";

function log(level: string, ...messages: string[]): void {
  const colors: { [key: string]: string } = {
    " DBG": "35",
    "INFO": "34",
    " ERR": "31",
    "WARN": "33",
  };

  let body = "";
  for (const message of messages) {
    body += ((typeof message === "string") ? message : util.inspect(message)) + " ";
  }

  const time = (new Date()).toTimeString().split(" ")[0];
  const color = colors[level] || "";
  console.log(`[\x1b[1;34msupervisor\x1b[0m ${time} \x1b[${color}m${level}\x1b[0m] ${body}`);
}

export function debug(...messages: any[]): void {
  log(" DBG", ...messages);
}

export function info(...messages: any[]): void {
  log("INFO", ...messages);
}

export function error(...messages: any[]): void {
  log(" ERR", ...messages);
}

export function warn(...messages: any[]): void {
  log("WARN", ...messages);
}
