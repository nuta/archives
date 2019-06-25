use crate::editor::Editor;

fn hover(editor: &mut Editor) {
    let mut win = editor.current().lock().unwrap();

}

pub fn activate(editor: &mut Editor) {
    editor.register_command("lsp.hover", "LSP: Hover", hover);
}