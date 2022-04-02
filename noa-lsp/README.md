# noa

A modern [nano](https://www.nano-editor.org/)-like terminal-based text editor.

- Native multiple cursors based editing inspired by [Visual Studio Code](https://code.visualstudio.com/).
- No distraction: let you focus on coding.
- Fuzzy file filder and global search.
- Smart auto completion and code navigation by [Langauge Server Protocol](https://microsoft.github.io/language-server-protocol/).
- Language-aware syntax highlighting and editing by [tree-sitter](https://tree-sitter.github.io/tree-sitter/).

## Installation

```
```

### How to Use

```bash
$ noa                   # opens the current directory.
$ noa ~/Projects/kerla  # Opens a workspace directory.
$ noa path/to/foo.c     # Opens a single file.
```

## Terminal Settings
To get noa to work perfectly, following features are required in your terminal:

- [Better key modifiers (CSI u)](https://iterm2.com/documentation-csiu.html) support *(optional)*. If you're using tmux, you'll need to update `tmux.conf` as well, see [documentation](https://github.com/tmux/tmux/wiki/Modifier-Keys#extended-keys).
- OSC52 (aka PASTE64) support *(optional)*: enables copying into the system clipboard from noa running on a remote host (e.g. connected over SSH).

## Keyboard Shortcuts
