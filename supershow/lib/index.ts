import * as main from "./main";

let activate, deactivate;
try {
    const vscode = require("./vscode");
    activate = vscode.activate;
    deactivate = vscode.deactivate;
} catch(e) {
}

export {
    main,
    activate,
    deactivate,
};
