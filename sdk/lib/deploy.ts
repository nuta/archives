import * as fs from "fs";
import * as JSZip from "jszip";
import * as path from "path";
import { api } from "./api";
import { loadAppYAML } from "./appdir";
import {
    find, generateTempPath, run, readTextFile, createFile, removeFiles
} from "./helpers";
import { logger } from "./logger";
import { FatalError } from "./types";
import { buildPlugin } from "./plugin_builder";
import { spawnSync } from "child_process";

async function mergeZipFiles(basepath: string, destZip: JSZip, srcZip: JSZip) {
    for (const filepath in srcZip.files) {
        const file = srcZip.files[filepath].async("arraybuffer");
        destZip.file(path.join(basepath, filepath), file);
    }

    return destZip;
}

async function loadZipData(zip: Buffer): Promise<JSZip> {
    return (new JSZip()).loadAsync(zip);
}

async function downloadAndExtractPackage(name: string, zip: JSZip, basepath: string) {
    logger.progress(`downloading \`${name}'`);
    const pluginZip = await api.downloadPlugin(name);

    logger.progress(`extracting \`${name}'`);
    zip = await mergeZipFiles(basepath, zip, await loadZipData(pluginZip));
    return zip;
}

async function populatePlugins(zip: JSZip, runtime: string,
    plugins: string[] | { [name: string]: string }): Promise<JSZip>
{
    zip = await downloadAndExtractPackage(runtime, zip, `node_modules/@makestack/${runtime}`);

    if (Array.isArray(plugins)) {
        const _plugins: {[name: string]: string} = {}
        for (const name of plugins) {
            _plugins[name] = 'latest';
        }
        plugins = _plugins;
    }

    for (const pluginName in plugins) {
        const url = plugins[pluginName];
        const basedir = `node_modules/@makestack/${pluginName}`;

        if (url.startsWith('file:')) {
            // Plugins in the local directory.
            const pluginZipPath = generateTempPath();

            logger.progress(`building \`${pluginName}'`);
            buildPlugin(url.split('file:')[1], pluginZipPath);

            logger.progress(`extracting \`${pluginName}'`);
            zip = await mergeZipFiles(`node_modules/@makestack/${pluginName}`, zip,
                await loadZipData(fs.readFileSync(pluginZipPath)));
            fs.unlinkSync(pluginZipPath);
        } else if (url === 'latest') {
            // Plugins on the GitHub.
            zip = await downloadAndExtractPackage(pluginName, zip, basedir);
            if (!zip.files[`node_modules/@makestack/${pluginName}/package.json`]) {
                zip.file(`node_modules/@makestack/${pluginName}/package.json`,
                    JSON.stringify({ name: pluginName, private: true }));
            }
        } else {
            throw new FatalError(`Unknown plugin version or url of \`${pluginName}': \`${url}'`)
        }
    }

    return zip;
}

export async function deploy(appYAML: any, files: any[]) {
    const appName = appYAML.name;
    const runtime = "runtime";
    const plugins = appYAML.plugins || [];
    let zip = new JSZip();
    let tempDir;

    // Populate plugin files.
    zip = await populatePlugins(zip, runtime, appYAML.plugins);

    // Copy start.js to the top level.
    logger.progress(`copying start.js from \`${runtime}'`);
    const startJsRelPath = `node_modules/@makestack/${runtime}/start.js`;
    if (!zip.files[startJsRelPath]) {
        throw new FatalError(`BUG: start.js is not found in \`${runtime}'.`);
    }
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

function shouldBePruned(filepath: string) {
    const EXCLUDED_EXTS = [
        '.md', '.cpp', '.ts', '.tgz', '.log'
    ]

    const EXCLUDED_FILENAMES = [
        'LICENSE', 'LICENSE.md', '.travis.yml', '.eslintrc.js', 'eslintignore',
        '.editorconfig', '.yarn-integrity', '.yarnclean', '.npmignore', '.gitignore',
        'Makefile', '.gitattributes', 'appveyor.yml', 'tsconfig.json', 'jsconfig.json',
        '.makestackignore', 'yarn.lock', 'license', '.eslintjsonrc.json', 'README.md',
        'CONTRIBUTING.md'
    ]

    const EXCLUDED_DIRS = [
        'test', 'tests', 'docs', 'doc', 'Documentation', 'examples', 'example',
        'coverage', '.vscode', '.idea', '.github', '.git'
    ]

    const { base, ext, dir } = path.parse(filepath);

    return EXCLUDED_EXTS.includes(ext) ||
        EXCLUDED_FILENAMES.includes(base) ||
        EXCLUDED_DIRS.some(name => filepath.match(`(node_modules\\/.+\\/)?${name}\\/`) !== null);
}

export async function deployAppDir(appDir: string) {
    const appYAML = loadAppYAML(appDir);
    const files = [];

    for (const filepath of find(appDir)) {
        if (!shouldBePruned(filepath) && !filepath.includes("node_modules")) {
            files.push({
                path: path.relative(appDir, filepath),
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

        for (const filepath of find(path.join(tempDir, 'node_modules'))) {
            if (!shouldBePruned(filepath)) {
                files.push({
                    path: path.relative(tempDir, filepath),
                    body: fs.readFileSync(filepath)
                });
            }
        }
    }

    await deploy(appYAML, files);

    if (tempDir) {
        removeFiles(tempDir);
    }
}
