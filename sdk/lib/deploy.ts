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
import { spawnSync } from "child_process";

export async function deploy(appYAML: any, files: any[]) {
    const appName = appYAML.name;
    const runtime = "runtime";
    let zip = new JSZip();
    let tempDir;

    // Copy app files.
    logger.progress(`copying files from \`${appName}' (${files.length} files)`);
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
            if (filepath.includes("/binding.gyp")) {
                const module = path.dirname(filepath);
                throw new FatalError(
                    `Deploying npm packages including native modules is not supported: ${module}`
                );
            }

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
