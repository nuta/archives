// The entry point in Firebase Functions.
require("babel-polyfill");
import * as path from "path";
import * as functions from "firebase-functions";

process.env.MAKESTACK_PRODUCTION = "1";
process.env.MAKESTACK_PLATFORM = "firebase";
process.env.APP_DIR = __dirname;
process.env.NODE_PATH = __dirname;

// Allow require("makestack") in the app's server program.
require("module").Module._initPaths();

const { getRuntimeInstance } = require(path.resolve(__dirname, "./makestack/dist/lib/platform/index"));
const platform = getRuntimeInstance();
platform.init();
export const makestack = functions.https.onRequest(platform.httpServer);
