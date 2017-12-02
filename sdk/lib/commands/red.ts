import * as bodyParser from "body-parser";
import * as chalk from "chalk";
import { fork, spawnSync } from "child_process";
import * as crypto from "crypto";
import * as express from "express";
import * as fs from "fs";
import * as proxy from "http-proxy-middleware";
import * as JSZip from "jszip";
import * as os from "os";
import * as path from "path";
import * as supervisor from "supervisor";
import { api } from "../api";
import { loadAppYAML } from "../appdir";
import { deploy } from "../deploy";
import { createFile, mkdirp } from "../helpers";

const NODE_RED_OFFICIAL_NODES_DIR = path.resolve(__dirname, "../../red");
const NODE_RED_DIR = path.resolve(os.homedir(), ".makestack/node-red/node-red");
const NODE_RED_USER_DIR = path.resolve(os.homedir(), ".makestack/node-red");
const NODE_RED_NODES_DIR = path.resolve(os.homedir(), ".makestack/node-red/nodes");
const NODE_RED_EMPTY_NODES_DIR = path.resolve(os.homedir(), ".makestack/node-red/nodes2");
const NODE_RED_TRANSPILERS_DIR = path.resolve(os.homedir(), ".makestack/node-red/transpilers");
const NODE_RED_URL = "https://github.com/node-red/node-red/archive/0.17.5.tar.gz";
const NODE_RED_PORT = 1880;
const NODE_RED_SETTINGS = `
module.exports = {
  uiPort: ${NODE_RED_PORT},
  uiHost: "127.0.0.1",
  debugMaxLength: 1000,
  coreNodesDir: "${NODE_RED_NODES_DIR}",
  nodesDir: "${NODE_RED_EMPTY_NODES_DIR}",
  logging: {
    console: {
      level: "info", // trace, debug, info, warn, error, or fatal
      metrics: false,
      audit: false
    }
  },
  editorTheme: {
    page: {
      title: "Node-RED for MakeStack",
    },
    header: {
      title: "Node-RED for MakeStack",
    },
    deployButton: {
      type:  "simple",
      label: "Deploy"
    }
  }
}
`;

function generateNodeScript(filepath) {
  const id = path.parse(filepath).name;
  const camelcase = id.replace(/-([a-z])/, (m, s) => s.toUpperCase());
  return `
module.exports = (RED) => {
  function ${camelcase}Node(config) {
    RED.nodes.createNode(this, config)
  }

  RED.nodes.registerType('${id}', ${camelcase}Node)
}
`;
}

function installNodeRED() {
  if (!fs.existsSync(path.join(NODE_RED_DIR, "red.js"))) {
    mkdirp(NODE_RED_DIR);
    spawnSync("curl", ["-LO", NODE_RED_URL], { cwd: NODE_RED_DIR });
    spawnSync("tar",
      ["xf", path.basename(NODE_RED_URL), "--strip-components", "1"],
      { stdio: "inherit", cwd: NODE_RED_DIR });
    spawnSync("npm", ["install"], { stdio: "inherit", cwd: NODE_RED_DIR });
    spawnSync("npx", ["grunt", "build"], { stdio: "inherit", cwd: NODE_RED_DIR });
  }

  fs.writeFileSync(path.join(NODE_RED_USER_DIR, "settings.js"), NODE_RED_SETTINGS);
}

function spawnNodeRED() {
  const nodeRedProcess = fork("red", ["-u", NODE_RED_USER_DIR], { cwd: NODE_RED_DIR });

  nodeRedProcess.on("close", () => {
    console.error(chalk.bold.red("Node-RED exited unexpectedly."));
    process.exit(1);
  });

  process.on("SIGTERM", () => process.exit(1));
  process.on("SIGINT", () => process.exit(1));
  process.on("exit", () => {
    nodeRedProcess.kill();
  });

  return nodeRedProcess;
}

function loadFlows(nodeRedJSON) {
  const app = JSON.parse(fs.readFileSync(nodeRedJSON, { encoding: "utf-8" }) || "{}");
  return { flows: app.flows };
}

function loadTranspilers() {
  const transpilers = [];

  // Official nodes.
  for (const filename of fs.readdirSync(NODE_RED_OFFICIAL_NODES_DIR)) {
    if (filename.match(/.js$/)) {
      transpilers[path.parse(filename).name] = require(`${NODE_RED_OFFICIAL_NODES_DIR}/${filename}`);
    }
  }

  // Plugin nodes.
  for (const filename of fs.readdirSync(NODE_RED_TRANSPILERS_DIR)) {
    transpilers[path.parse(filename).name] = require(path.join(NODE_RED_TRANSPILERS_DIR, filename));
  }

  return transpilers;
}

function generateNodeId(id) {
  const hash = crypto.createHash("sha1").update(id).digest("hex").substring(0, 16);
  return `__${hash}__`;
}

function transpile(flows) {
  const transpilers = loadTranspilers();
  const loadedModules = ["events"];
  const loadedPlugins = [];

  console.log(require("util").inspect(flows, false, 8));

  let code = `
    const EventEmitter = require('events')
    let __nodes__ = {}

    function __next__(nodeId, outputs) {
      for (const [index, output] of outputs.entries()) {
        for (const nextNodeId of __nodes__[nodeId].outputs[index]) {
          __nodes__[nextNodeId].ev.emit('input', output)
        }
      }
    }

    function __initialize_nodes__() {
  `;

  for (const flow of flows) {
    if (flow.type === "tab") {
      continue;
    }

    const nodeId = generateNodeId(flow.id);
    flow.nodeId = nodeId;
    const { type: nodeType, init, oninput, modules, plugins } = transpilers[flow.type](flow);
    const outputs = JSON.stringify(flow.wires.map((wire) => wire.map(generateNodeId)));

    let nodeCode = "";
    for (const moduleName of modules || []) {
      if (!(moduleName in loadedModules)) {
        nodeCode += `const ${moduleName} = require('${moduleName}')`;
        loadedModules.push(moduleName);
      }
    }

    for (const pluginName of plugins || []) {
      if (!(pluginName in loadedPlugins)) {
        nodeCode += `const ${pluginName} = require('@makestack/${pluginName}')`;
        loadedPlugins.push(pluginName);
      }
    }

    nodeCode += `
      const ev_${nodeId} = new EventEmitter()
      __nodes__['${nodeId}'] = {
        ev: ev_${nodeId},
        type: '${nodeType}',
        outputs: ${outputs}
      }

      ${init}
      ev_${nodeId}.on('input', __input__ => {
        ${oninput}
      })

    `;

    nodeCode = nodeCode.replace("__outputs__()", `__next__('${nodeId}')`);
    nodeCode = nodeCode.replace("__outputs__(", `__next__('${nodeId}', `);
    code += nodeCode;
  }

  code += `
    }

    __initialize_nodes__()
  `;

  console.log(code);
  return code;
}

async function deployFlows(appDir, nodeRedJSON, body) {
  const nodeREDJSON = {
    flows: body.flows,
  };

  fs.writeFileSync(nodeRedJSON, JSON.stringify(nodeREDJSON, null, 2));
  const script = transpile(body.flows);
  await deploy(loadAppYAML(appDir), [{ path: "app.js", body: script }]);
}

function spawnProxyServer(port, appDir, nodeRedJSON) {
  const server = express();
  server.use(bodyParser.json());

  server.get("/flows", (req, res) => {
    /* Return saved Node-RED app flows. */
    res.send(loadFlows(nodeRedJSON));
  });

  server.post("/flows", (req, res) => {
    /* Save aand Deploy the app. */
    try {
      deployFlows(appDir, nodeRedJSON, req.body)
        .then((r) => res.status(200).send(r))
        .catch((e) => {
          console.error(e);
          res.status(500).send(e);
        });
    } catch (e) {
      console.error(e);
      res.status(500).send(e);
    }
  });

  server.use("/", proxy({
    target: `http://localhost:${NODE_RED_PORT}`,
    changeOrigin: true,
    ws: true,
  }));

  server.listen(port, () => {
    console.log(chalk.bold.green(`Open http://localhost:${port} on your web browser!`));
  });
}

function streamAppLog(appName, nodeRedProcess) {
  api.streamAppLog(appName, (lines) => {
    for (const line of lines) {
      nodeRedProcess.send({ type: "log", log: line });
    }
  });
}

export async function main(args, opts, logger) {
  if (opts.dev) {
    const argv = process.argv.slice(1).filter((arg) => arg !== "--dev");

    supervisor.run(["-w", path.resolve(__dirname, "../../lib") + "," +
      path.resolve(NODE_RED_DIR, ".makestack/node-red") + "," +
      path.resolve(__dirname, "../../../plugins"),
      "-e", "js,html", "--", ...argv]);
    return;
  }

  const appYAML = loadAppYAML(opts.appDir);

  mkdirp(NODE_RED_USER_DIR);
  mkdirp(NODE_RED_NODES_DIR);
  mkdirp(NODE_RED_EMPTY_NODES_DIR);
  mkdirp(NODE_RED_TRANSPILERS_DIR);

  // Official plugins.
  for (const filepath of fs.readdirSync(NODE_RED_OFFICIAL_NODES_DIR)) {
    if (filepath.match(/\.js$/)) {
      createFile(`${NODE_RED_TRANSPILERS_DIR}/${path.basename(filepath)}`,
        fs.readFileSync(path.join(NODE_RED_OFFICIAL_NODES_DIR, filepath)));
    }

    if (filepath.match(/\.html$/)) {
      createFile(`${NODE_RED_NODES_DIR}/${path.basename(filepath)}`,
        fs.readFileSync(path.join(NODE_RED_OFFICIAL_NODES_DIR, filepath)));
      createFile(`${NODE_RED_NODES_DIR}/${path.parse(filepath).name}.js`,
        generateNodeScript(filepath));
    }
  }

  // Download plugins.
  for (const pluginName of appYAML.plugins || []) {
    const pluginZip = await (new JSZip()).loadAsync(await api.downloadPlugin(`nodejs-${pluginName}`));
    for (const filepath in pluginZip.files) {
      if (filepath.startsWith("red/") && filepath.match(/\.js$/)) {
        createFile(`${NODE_RED_TRANSPILERS_DIR}/${path.basename(filepath)}`,
          await pluginZip.files[filepath].async("string"));
      }

      if (filepath.startsWith("red/") && filepath.match(/\.html$/)) {
        createFile(`${NODE_RED_NODES_DIR}/${path.basename(filepath)}`,
          await pluginZip.files[filepath].async("string"));
        createFile(`${NODE_RED_NODES_DIR}/${path.parse(filepath).name}.js`,
          generateNodeScript(path.basename(filepath)));
      }
    }
  }

  const nodeRedJSON = path.resolve(opts.appDir, "red.json");
  if (!fs.existsSync(nodeRedJSON)) {
    fs.writeFileSync(nodeRedJSON, JSON.stringify({
      flows: [{ id: "fa5ac137.92397", type: "tab", label: "Flow 1" }],
    }));
  }
  logger.info("==> Setting up Node-RED");
  installNodeRED();
  logger.info("==> Starting Node-RED");
  const nodeRedProcess = spawnNodeRED();
  logger.info("==> Stream app log");
  streamAppLog(appYAML.name, nodeRedProcess);
  logger.info("==> Starting a proxy server");
  spawnProxyServer(opts.port, opts.appDir, nodeRedJSON);
}
