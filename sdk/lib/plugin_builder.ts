import { spawnSync } from "child_process";
import * as fs from "fs";
import * as os from "os";
import * as path from "path";
import { FatalError } from "./types";
import { logger } from "./logger";

function buildDockerImage() {
    logger.progress('building a Docker image');

    const { error, status } = spawnSync("docker", ["build", "-t", "makestack/plugin-builder", "."], {
        stdio: "inherit",
        env: process.env, // DOCKER_HOST, etc.
        cwd: path.resolve(__dirname, "../plugin_builder"),
    });

    if (error && (error as any).code === 'ENOENT') {
        throw new FatalError("`docker' command is not installed.");
    }

    if (status !== 0) {
        throw new FatalError(`docker build exited with ${status}`)
    }
}

function runContainer(pluginDir: string) {
    logger.progress('building the plugin');
    const tempdir = path.join(os.homedir(), ".makestack", "plugin-builder");

    // Note that docker-machine in macOS does not support mounting volumes
    // outside the home directory.
    const { status } = spawnSync("docker",
        [
            "run", "--rm", "-v", `${path.resolve(pluginDir)}:/plugin:ro`,
            "-v", `${tempdir}:/dist`, "-t", "makestack/plugin-builder"
        ], {
        stdio: "inherit",
        env: process.env, // DOCKER_HOST, etc.
    });

    if (status !== 0) {
        throw new FatalError(`docker run exited with ${status}`)
    }

    return path.resolve(tempdir, "plugin.zip")
}

export function buildPlugin(pluginDir: string, destPath: string) {
    buildDockerImage();
    const pluginZip = runContainer(pluginDir);
    fs.copyFileSync(pluginZip, destPath);
}
