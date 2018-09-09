import * as fs from "fs";
import * as path from "path";
import * as mustache from "mustache";
import * as vscode from "vscode";
import { render } from "./render";

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
        let rendered;
        let error;
        try {
            rendered = render(doc.getText());
        } catch (e) {
            error = e.stack;
        }

        let html;
        if (rendered) {
            html = mustache.render(this.template, {
                title: rendered.front.title || "No title",
                theme: rendered.front.theme || "simple",
                body: rendered.htmlByLine(editor.selection.active.line),
                csp: [
                    "default-src 'none'",
                    "img-src    vscode-resource: https: file:",
                    "script-src vscode-resource: https: 'unsafe-inline'",
                    "style-src  vscode-resource: https: 'unsafe-inline'",
                    "font-src   vscode-resource: https:",
                ].join(";")
            });
        } else {
            html = "<html><body><pre>" + error + "</pre></body></html>";
        }

        this.panel.webview.html = html;
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
