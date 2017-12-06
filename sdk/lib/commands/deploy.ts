const { deployAppDir } = require("../deploy");

export async function main(args: any, opts: any, logger: any) {
    await deployAppDir(opts.appDir);
}
