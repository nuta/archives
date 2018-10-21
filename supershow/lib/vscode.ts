import * as fs from "fs";
import * as path from "path";
import * as vscode from "vscode";
import { renderHtml } from "./render";

class SupershowController {
    private disposable: vscode.Disposable;
    private panel: vscode.WebviewPanel;
    private template: string;

    constructor() {
        this.template = fs.readFileSync(
            path.resolve(__dirname, "../dist/ui/index.html"),
            { encoding: "utf-8" }
        );

        this.panel = vscode.window.createWebviewPanel(
            "supershow-preview",
            "Supershow",
            vscode.ViewColumn.Two,
            {}
        );

        const subscriptions: vscode.Disposable[] = [];
        vscode.workspace.onDidChangeTextDocument(this.onChangeText, this, subscriptions);
        vscode.window.onDidChangeTextEditorSelection(this.onChangeText, this, subscriptions);

        this.disposable = vscode.Disposable.from(...subscriptions);
        this.onChangeText();
    }

    private onChangeText() {
        const editor = vscode.window.activeTextEditor;
        if (!editor) {
            return;
        }

        const doc = editor.document;
        if (doc.languageId !== "markdown") {
            return;
        }

        this.render(editor, doc);
    }

    private render(editor: vscode.TextEditor, doc: vscode.TextDocument) {
        const md = doc.getText();
        const line = editor.selection.active.line;
        const csp = [
            "default-src 'none'",
            "img-src    vscode-resource: https: file:",
            "script-src vscode-resource: https: 'unsafe-inline'",
            "style-src  vscode-resource: https: 'unsafe-inline'",
            "font-src   vscode-resource: https:",
        ];
        try {
            this.panel.webview.html = renderHtml(md, line, { csp: csp.join(";") }).html;
        } catch (e) {
            this.panel.webview.html = "<html><body><pre>" + e.stack + "</pre></body></html>";
        }
    }
}

export function activate(context: vscode.ExtensionContext) {
    const previewCommand = vscode.commands.registerCommand("supershow.preview", () => {
        new SupershowController();
    });

    context.subscriptions.push(previewCommand);
}

export function deactivate() {
}
