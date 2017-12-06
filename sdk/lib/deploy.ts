import * as fs from "fs";
import * as JSZip from "jszip";
import * as path from "path";
import { api } from "./api";
import { loadAppYAML } from "./appdir";
import {
    find, generateTempPath, run, readTextFile, createFile, removeFiles
} from "./helpers";
import { logger } from "./logger";
import { spawnSync } from "child_process";

async function mergeZipFiles(basepath: string, destZip: JSZip, srcZip: JSZip) {
    for (const filepath in srcZip.files) {
        const file = srcZip.files[filepath].async("arraybuffer");
        destZip.file(path.join(basepath, filepath), file);
    }

    return destZip;
}

async function downloadAndExtractPackage(name: string, zip: JSZip, basepath: string) {
    logger.progress(`downloading \`${name}'`);
    const pluginZip = await api.downloadPlugin(name);

    logger.progress(`extracting \`${name}'`);
    zip = await mergeZipFiles(basepath, zip, await (new JSZip()).loadAsync(pluginZip));
    return zip;
}

export async function deploy(appYAML: any, files: any[]) {
    const appName = appYAML.name;
    const runtime = "runtime";
    const plugins = appYAML.plugins || [];
    let zip = new JSZip();
    let tempDir;

    // Download the runtime.
    zip = await downloadAndExtractPackage(runtime, zip, `node_modules/@makestack/${runtime}`);

    // Populate plugin files.
    for (const pluginName of plugins) {
        zip = await downloadAndExtractPackage(pluginName, zip, `node_modules/@makestack/${pluginName}`);
        if (!zip.files[`node_modules/@makestack/${pluginName}/package.json`]) {
            zip.file(`node_modules/@makestack/${pluginName}/package.json`,
            JSON.stringify({ name: pluginName, private: true }));
        }
    }

    // Copy start.js to the top level.
    logger.progress(`copying start.js from \`${runtime}'`);
    const startJsRelPath = `node_modules/@makestack/${runtime}/start.js`;
    zip.file("start.js", zip.files[startJsRelPath].async("arraybuffer"));

    // Copy app files.
    logger.progress(`copying files from \`${appName}' (${files.length} files including npm packages)`);
    for (const file of files) {
        logger.debug(`adding \`${file.path}' (${file.body.length} bytes)`);
        zip.file(file.path, file.body);
    }

    logger.progress("generating zip file");
    const appImage = await zip.generateAsync({
        type: "nodebuffer",
        compression: "DEFLATE",
        compressionOptions: {
            level: 9,
        },
    }) as Buffer;

    logger.progress(`deploying (${(appImage.length / 1024).toFixed(0)} KB)`);
    const deployment = await api.deploy(appName, appImage);
    logger.success(`Successfully deployed version #${deployment.version}!`);
}

export async function deployAppDir(appDir: string) {
    const EXCLUDED_FILES_PATTERN = /(node_modules|yarn\.lock|app\.yaml|jsconfig.json|yarn-error.log|README\.md)/
    const appYAML = loadAppYAML(appDir);
    const files = [];

    for (const filepath of find(appDir)) {
        if (!filepath.match(EXCLUDED_FILES_PATTERN)) {
            files.push({
                path: filepath,
                body: readTextFile(filepath),
            });
        }
    }

    // Download npm dependencies.
    let tempDir;
    if (fs.existsSync(path.join(appDir, "package.json")) &&
        fs.existsSync((path.join(appDir, "yarn.lock")))) {
        tempDir = generateTempPath()
        fs.mkdirSync(tempDir)
        fs.copyFileSync(path.join(appDir, "package.json"), path.join(tempDir, "package.json"))
        fs.copyFileSync(path.join(appDir, "yarn.lock"), path.join(tempDir, "yarn.lock"))

        logger.progress("downloading npm dependencies...")
        run(["yarn", "install", "--production"], { cwd: tempDir })

        for (const filepath of find(path.join())) {
            files.push({ path: filepath, body: fs.readFileSync(filepath) });
        }
    }

    await deploy(appYAML, files);

    if (tempDir) {
        removeFiles(tempDir);
    }
}
