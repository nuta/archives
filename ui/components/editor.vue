<template>
  <div class="editor" ref="editor"></div>
</template>

<script>
import bundledTypeDefs from "~/assets/dts.json"
import colorSchemes from "~/assets/color-schemes.json"
import { getCurrentTheme } from "~/assets/js/preferences";
import * as monaco from '@timkendrick/monaco-editor';

export default {
  props: ['code'],
  watch: {
    code() {
      // `code' prop could be changed after mounted hook.
      if (this.editor && this.editor.getModel().getValue() === '' && this.code !== '') {
        this.editor.getModel().setValue(this.code)
      }
    }
  },
  data() {
    return {
      prevCode: null
    }
  },
  methods: {
    setupMonacoEditor(typeDefs, colorSchemes) {
      monaco.languages.typescript.javascriptDefaults.
        setCompilerOptions({
          allowJs: true,
          target: monaco.languages.typescript.ScriptTarget.ES2017,
          moduleResolution: monaco.languages.typescript.ModuleResolutionKind.  NodeJs,
          module: monaco.languages.typescript.ModuleKind.CommonJS,
          noLib: true,
          allowNonTsExtensions: true,
          typeRoots: ["node_modules/@types"]
        });

      this.editorExtraLibs = []
      for (const { name, body } of typeDefs) {
        this.editorExtraLibs.push(
          monaco.languages.typescript.javascriptDefaults.addExtraLib(body, name)
        )
      }

      for (const [name, colorScheme] of Object.entries(colorSchemes)) {
        monaco.editor.defineTheme(name, colorScheme)
      }

      this.editor = monaco.editor.create(document.querySelector('.editor'), {
        theme: getCurrentTheme(),
        fontFamily: 'Source Code Pro',
        fontSize: '15px',
        automaticLayout: true,
        autoIndent: true,
  	    value: this.code,
        language: 'javascript',
        minimap: {
          enabled: false
        }
      })
    },

    async downloadExternalTypeDefs() {
      return [
        {
          name: 'node',
          body: require('~/node_modules/@types/node/index.d.ts')
        }
      ]
    },
    defineShortcuts() {
        this.editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.KEY_S, () => {
          // TODO
        })

        this.editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.KEY_P, () => {
          this.editor.getAction('editor.action.quickCommand').run()
        })
    },
    registerOnchangeHandler() {

      this.editor.onDidChangeModelContent(() => {
          // Backup in case we hit a bug.
          const code = this.editor.getModel().getValue()
          if (code.length > 0) {
            localStorage.setItem('backup', code)
          }

          if (!this.prevCode) {
            // Initial value is being set.
            this.prevCode = code
            return
          }

          this.$emit('changed', code)
          this.prevCode = code
        })
    },
    async initializeMonacoEditor() {
      const externalTypeDefs = await this.downloadExternalTypeDefs()
      const typeDefs = [
        ...bundledTypeDefs,
        ...externalTypeDefs
      ]

      this.setupMonacoEditor(typeDefs, colorSchemes)
      this.defineShortcuts()
      this.editor.getModel().setValue(this.code)
      this.registerOnchangeHandler()
    },
  },
  async mounted() {
    await this.initializeMonacoEditor()

  },
  beforeDestroy() {
    if (Array.isArray(this.editorExtraLibs)) {
      this.editorExtraLibs.map(lib => lib.dispose())
    }

    if (this.editor) {
      this.editor.dispose()
    }
  }
}
</script>

<style lang="scss" scoped>
.editor {
  display: flex;
  flex: 1;
  z-index: 20000;
}
</style>
