import * as fs from "fs";
import * as yaml from "js-yaml";
import * as path from "path";
import { FatalError } from "./types";

export function loadPackageJson(appDir: string) {
  if (!fs.existsSync(appDir)) {
    throw new FatalError(`invalid app dir: ${appDir}`);
  }

  const packageJsonPath = path.join(appDir, "package.json");
  if (!fs.existsSync(packageJsonPath)) {
    throw new FatalError(`package.json not found: ${packageJsonPath}`);
  }

  const body = fs.readFileSync(packageJsonPath, { encoding: 'utf-8' });
  return JSON.parse(body) || {};
}
