// The entry point in Firebase Functions.
require("babel-polyfill");
import * as path from "path";
import * as functions from "firebase-functions";

process.env.APP_DIR = __dirname;
const { getRuntimeInstance } = require(path.resolve(__dirname, "./makestack/dist/lib/runtime"));
const platform = getRuntimeInstance();
platform.start();
export const api = functions.https.onRequest(platform.httpServer);
