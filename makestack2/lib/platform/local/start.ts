import { getRuntimeInstance } from "..";
import { logger } from "../../logger";
import { LocalPlatformRuntime } from "./runtime";

function main() {
    const host = process.env.HOST || "0.0.0.0";
    const port = parseInt(process.env.PORT || "7878");

    const platform = getRuntimeInstance() as LocalPlatformRuntime;
    platform.init();
    platform.start(host, port);

    logger.debug(`Started a http server at http://${host}:${port}`);
}

main();
