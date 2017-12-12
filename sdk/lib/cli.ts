import * as addDeviceCommand from "./commands/add_device";
import * as appCommand from "./commands/app";
import * as configCommand from "./commands/config";
import * as buildPluginCommand from "./commands/build_plugin";
import * as deployCommand from "./commands/deploy";
import * as deployImageCommand from "./commands/deploy_image";
import * as deviceCommand from "./commands/device";
import * as deviceConfigCommand from "./commands/device_config";
import * as installCommand from "./commands/install";
import * as listDrivesCommand from "./commands/list_drives";
import * as loginCommand from "./commands/login";
import * as mockCommand from "./commands/mock";
import * as newCommand from "./commands/new";
import * as newPluginCommand from "./commands/new_plugin";
import * as prepareCommand from "./commands/prepare";
import * as redCommand from "./commands/red";
import * as replCommand from "./commands/repl";
import * as runCommand from "./commands/run";
import * as fs from "fs";
import * as path from "path";
const program = require("caporal");

program
.version(
    JSON.parse(fs.readFileSync(
        path.resolve(__dirname, '../package.json'), { encoding: 'utf-8' }
    )).version,
)
.description('A MakeStack CLI Developer Tools.');

program
.command("login", "Get an access credential for an MakeStack Server account.")
.action(loginCommand.main);

program
.command("app list", "List apps.")
.action(appCommand.list);

program
.command("app create", "Create an app.")
.argument("name", "The app name.")
.option("--api <api>", "The API.", /^nodejs$/, null, true)
.action(appCommand.create);

program
.command("app delete", "Delete an app.")
.argument("name", "The app name.")
.action(appCommand.delete_);

program
.command("config list", "List app configs.")
.option("--app-dir <app-dir>", "The app directory.", null, process.cwd())
.action(configCommand.list);

program
.command("config set", "Create/Update an app config.")
.argument("name", "The config name.")
.argument("value", "The value.")
.option("--type <type>", "The value data type. (string, integer, float, or bool)", /string|integer|float|bool/, null, true)
.option("--app-dir <app-dir>", "The app directory.", null, process.cwd())
.action(configCommand.set);

program
.command("config delete", "Delete an app config.")
.argument("name", "The config name.")
.option("--app-dir <app-dir>", "The app directory.", null, process.cwd())
.action(configCommand.delete_);

program
.command("device list", "List devices.")
.action(deviceCommand.list);

program
.command("device delete", "Delete an device.")
.argument("name", "The device name.")
.action(deviceCommand.delete_);

program
.command("device-config list", "List device-local configs.")
.argument("--device-name <device-name>", "The device name.")
.action(deviceConfigCommand.list);

program
.command("device-config set", "Create/Update an device-local config.")
.option("--device-name <device-name>", "The device name.", null, null, true)
.option("--type <type>", "The value data type. (string, integer, float, or bool)", /string|integer|float|bool/, null, true)
.argument("name", "The config name.")
.argument("value", "The value.")
.action(deviceConfigCommand.set);

program
.command("device-config delete", "Delete an device-local config.")
.argument("--device-name <device-name>", "The device name.")
.argument("name", "The config name.")
.action(deviceConfigCommand.delete_);

program
.command("deploy", "Deploy the app.")
.option("--app-dir <app-dir>", "The app directory.", null, process.cwd())
.action(deployCommand.main);

program
.command("deploy-image", "Deploy the app with a prebuilt image.")
.option("--app <app>", "The app name.", null, process.cwd())
.argument("image", "The prebuilt image.")
.action(deployImageCommand.main);

program
.command("add-device", "Add an device to the app.")
.argument("name", "The device name.")
.option("--app-dir <app-dir>", "The app directory.", null, process.cwd())
.action(addDeviceCommand.main);

program
.command("run", "Run as a device.")
.option("--device-id <device-id>", "The device ID.", null, null, true)
.option("--device-secret <device-secret>", "The device secret.", null, null, true)
.option("--adapter <adapter>", "The network adapter.", /^http|sakuraio$/, "http")
.option("--server <url>", "The server url.", /^(http|https):\/\//, null, false)
.option("--heartbeat-interval <seconds>", "The server url.", null, 15)
.action(runCommand.main);

program
.command("mock create", "Create a mock device.")
.argument("name", "The mock device name.")
.option("--device-type <device-type>", "The device type.", /^raspberrypi3|sdk$/, "sdk")
.action(mockCommand.create);

program
.command("mock run", "Run a mock device.")
.argument("name", "The mock device name.")
.option("--adapter <adapter>", "The network adapter.", /^http|sakuraio$/, "http")
.action(mockCommand.run);

program
.command("repl", "A Read-Eval-Print-Loop (REPL) console.")
.argument("name", "The device name.")
.action(replCommand.main);

program
.command("list-drives", "List available drives to install MakeStack OS/Linux.")
.action(listDrivesCommand.main);

function wifiCountryNameValidator(code: string) {
    return code in [
        "AF", "AX", "AL", "DZ", "AS", "AD", "AO", "AI", "AQ", "AG", "AR", "AM", "AW", "AU", "AT", "AZ", "BS", "BH", "BD",
        "BB", "BY", "BE", "BZ", "BJ", "BM", "BT", "BA", "BW", "BV", "BR", "IO", "BN", "BG", "BF", "BI", "KH", "CM", "CA",
        "CV", "KY", "CF", "TD", "CL", "CN", "CX", "CC", "CO", "KM", "CG", "CK", "CR", "CI", "HR", "CU", "CW", "CY", "CZ",
        "DK", "DJ", "DM", "DO", "EC", "EG", "SV", "GQ", "ER", "EE", "ET", "FK", "FO", "FJ", "FI", "FR", "GF", "PF", "TF",
        "GA", "GM", "GE", "DE", "GH", "GI", "GR", "GL", "GD", "GP", "GU", "GT", "GG", "GN", "GW", "GY", "HT", "HM", "VA",
        "HN", "HK", "HU", "IS", "IN", "ID", "IQ", "IE", "IM", "IL", "IT", "JM", "JP", "JE", "JO", "KZ", "KE", "KI", "KW",
        "KG", "LA", "LV", "LB", "LS", "LR", "LY", "LI", "LT", "LU", "MO", "MG", "MW", "MY", "MV", "ML", "MT", "MH", "MQ",
        "MR", "MU", "YT", "MX", "MC", "MN", "ME", "MS", "MA", "MZ", "MM", "NA", "NR", "NP", "NL", "NC", "NZ", "NI", "NE",
        "NG", "NU", "NF", "MP", "NO", "OM", "PK", "PW", "PA", "PG", "PY", "PE", "PH", "PN", "PL", "PT", "PR", "QA", "RE",
        "RO", "RU", "RW", "BL", "KN", "LC", "MF", "PM", "VC", "WS", "SM", "ST", "SA", "SN", "RS", "SC", "SL", "SG", "SX",
        "SK", "SI", "SB", "SO", "ZA", "GS", "SS", "ES", "LK", "SD", "SR", "SJ", "SZ", "SE", "CH", "SY", "TJ", "TH", "TL",
        "TG", "TK", "TO", "TT", "TN", "TR", "TM", "TC", "TV", "UG", "UA", "AE", "GB", "US", "UM", "UY", "UZ", "VU", "VN",
        "WF", "EH", "YE", "ZM", "ZW", "BO", "BQ", "CD", "IR", "KP", "KR", "MK", "FM", "MD", "PS", "SH", "TW", "TZ", "VE",
        "VG", "VI",
    ];
}

program
.command("install", "Install MakeStack OS/Linux to the device.")
.option("--name <name>", "The device name.", null, null, true)
.option("--type <type>", "The device type.", /^raspberrypi3|mock$/, null, true)
.option("--os <os>", "The OS image.", /^linux$/, "linux")
.option("--drive <drive>", "The drive. Use `list-drives' command.", null, null, true)
.option("--adapter <adapter>", "The network adapter.", /^http|sakuraio$/, "http")
.option("--ignore-duplication", "Ignore duplicated device name.", null, false)
.option("--wifi-ssid", "The Wi-Fi SSID.", null, null, false)
.option("--wifi-password", "The Wi-Fi password. (WPA/WPA2 Personal)", null, null, false)
.option("--wifi-country", "The Wi-Fii country code. (ISO/IEC alpha2 country code)", wifiCountryNameValidator, null, false)
.action(installCommand.main);

program
.command("build-plugin", "Build a plugin using Docker.")
.option("--plugin-dir", "The plugin directory.", null, process.cwd(), false)
.option("--outfile", "The output zip file path.", null, path.basename(process.cwd()) + ".zip", false)
.action(buildPluginCommand.main);

program
.command("prepare", "Prepare a app/plugin development.")
.option("--app-dir <app-dir>", "The app directory.", null, process.cwd())
.action(prepareCommand.main);

program
.command("new", "Create an app directory with template files.")
.argument("dir", "The plugin directory.")
.option("--plugins <plugins>", "A list of plugins separated by `,'.", null, "")
.action(newCommand.main);

program
.command("new-plugin", "Create a plugin directory with template files.")
.argument("dir", "The plugin directory.")
.option("--lang <lang>", "The programming language.", /^typescript|javascript$/, "typescript")
.action(newPluginCommand.main);

program
.command("red", "Program the app with Node-RED.")
.option("--port <port>", "The port.", null, 31313)
.option("--app-dir <app-dir>", "The app directory.", null, process.cwd())
.option("--dev", "Restart Node-RED automatically for Node-RED node development.", false)
.action(redCommand.main);

export async function run(argv: string[]): Promise<{}> {
    return new Promise((resolve, reject) => {
        program.fatalError = reject;
        program.parse(argv).then(resolve);
    });
}
