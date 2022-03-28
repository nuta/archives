// The entry point in Firebase.
import * as path from "path";
import * as express from "express";
const functions = require("firebase-functions");

// XXX:
const { initServer } = require(
    path.join(__dirname, "makestack/dist/server/start"));
const { ProtocolServer } = require(
    path.join(__dirname, "makestack/dist/server/index"));

const eventCallback = (name: string, value: any) => {
    const callback = (global as any).__eventEndpoints[name];
    if (callback) {
        callback(value);
    }
};

const protocolServer = new ProtocolServer(path.resolve(__dirname, "firmware.bin"), eventCallback);
const httpServer = express();
initServer(path.resolve(__dirname, "app.js"), httpServer);

httpServer.post("/makestack/protocol", (req, res) => {
    // req.rawBody is available in the Cloud Functions environment:
    // https://firebase.google.com/docs/functions/http-events#read_values_from_the_request
    const rawBody: Buffer = (req as any).rawBody;
    let reply = protocolServer.processPayload(rawBody);
    reply = reply ? reply : protocolServer.buildHeartbeatPayload();
    res.status(200);
    res.type("application/octet-stream");
    res.send(reply);
});

export const api = functions.https.onRequest(httpServer);
