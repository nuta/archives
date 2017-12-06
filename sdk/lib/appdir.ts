import * as fs from "fs";
import * as yaml from "js-yaml";
import * as path from "path";
import { FatalError } from "./types";

export function loadAppYAML(appDir: string) {
  if (!fs.existsSync(appDir)) {
    throw new FatalError(`invalid app dir: ${appDir}`);
  }

  const appYAMLPath = path.join(appDir, "app.yaml");
  if (!fs.existsSync(appYAMLPath)) {
    throw new FatalError(`app.yaml not found: ${appYAMLPath}`);
  }

  return yaml.safeLoad(fs.readFileSync(appYAMLPath, { encoding: 'utf-8' })) || {};
}
