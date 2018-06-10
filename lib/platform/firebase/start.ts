// Firebase Functions loads this file.
import * as fs from "fs";
import * as path from "path";
require("babel-polyfill");

const libDir = path.resolve(__dirname, "./makestack");

// Add __dirname to allow requiring libDir.
process.env.NODE_PATH = __dirname;
require("module").Module._initPaths();

process.env.APP_DIR = __dirname;
const mainPath = path.join(libDir, "./dist/lib/platform/firebase/main");
export const makestack = require(mainPath).api;
