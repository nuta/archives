import * as fs from "fs";
import * as path from "path";
import * as vscode from "vscode";
import { render } from "./render";

class SupershowController {
    private disposable: vscode.Disposable;
    private panel: vscode.WebviewPanel;

    constructor() {
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
        let html = fs.readFileSync(
            path.resolve(__dirname, "../ui/index.vscode.html"),
            { encoding: "utf-8" }
        );

        const md = doc.getText();

        let rendered;
        let error;
        let mdHtml;
        try {
            rendered = render(md);
        } catch (e) {
            error = e.stack;
        }

        if (rendered) {
            mdHtml = rendered.htmlByLine(editor.selection.active.line);
        } else {
            mdHtml = "<html><body><pre>" + error + "</pre></body></html>";
        }


        html = html.replace("__HTML__", mdHtml);
        this.panel.webview.html = html;
    }
}

export function activate(context: vscode.ExtensionContext) {
    console.info("supershow: activate!");
    const previewCommand = vscode.commands.registerCommand("supershow.preview", () => {
        new SupershowController();
    });

    context.subscriptions.push(previewCommand);
}

export function deactivate() {
}
