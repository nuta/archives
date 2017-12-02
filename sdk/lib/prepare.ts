import { spawnSync } from "child_process";
import * as fs from "fs";
import * as yaml from "js-yaml";
import * as JSZip from "jszip";
import * as path from "path";
import { api } from "./api";
import { createFile } from "./helpers";
import { logger } from "./logger";

function installDevDependencies(dir: string) {
  const { status, stdout, stderr } = spawnSync("yarn", [], { encoding: "utf-8", cwd: dir });
  if (status !== 0) {
    throw new Error(`yarn exited with ${status}:\n${stdout}\n${stderr}`);
  }
}

async function downloadAndExtractPlugin(plugin: string, dir: string) {
  const pluginZip = await (new JSZip()).loadAsync(await api.downloadPlugin(plugin));

  for (const filepath in pluginZip.files) {
    if (!filepath.endsWith("/")) {
      const body = await pluginZip.files[filepath].async("nodebuffer");
      createFile(path.join(dir, "node_modules/@makestack", plugin, filepath), body);
    }
  }
}

export async function prepare(dir) {
  const makestackTypeFile = path.resolve(__dirname, "../../runtime/makestack.d.ts");

  let yamlPath;
  if (fs.existsSync(path.join(dir, "app.yaml"))) {
    yamlPath = path.join(dir, "app.yaml");
  } else if (fs.existsSync(path.join(dir, "plugin.yaml"))) {
    yamlPath = path.join(dir, "plugin.yaml");
  } else {
    throw new Error("The current directory is not an app nor a plugin.");
  }

  if (fs.existsSync(path.join(dir, "package.json"))) {
    logger.progress("Installing npm dev dependecies");
    installDevDependencies(dir);
  }

  const { plugins } = yaml.safeLoad(fs.readFileSync(yamlPath));
  for (const plugin of plugins || []) {
    if (fs.existsSync(path.join(dir, "node_modules/@makestack", plugin))) {
      logger.progress(`Download @makestack/${plugin} (alredy exists)`);
    } else {
      logger.progress(`Download @makestack/${plugin}`);
      await downloadAndExtractPlugin(plugin, dir);
    }
  }

  logger.progress("Adding @types/makestack");
  createFile(path.join(dir, "node_modules/@types/makestack/index.d.ts"),
    fs.readFileSync(makestackTypeFile));
}
