<template>
  <div class="editor" ref="editor"></div>
</template>

<script>
import bundledTypeDefs from "~/assets/dts.json"
import colorSchemes from "~/assets/color-schemes.json"
import { getCurrentTheme } from "~/assets/js/preferences";
import 'whatwg-fetch'

const CDN_URL = 'https://cdnjs.cloudflare.com/ajax/libs/monaco-editor/0.10.1/min'
const DEFINITELY_TYPED_REV = 'ea2dc4dc2831c32fe96aa8bb225a42c7b2c9193a'

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
    loadMonacoEditor() {
      window.MonacoEnvironment = {
    		getWorkerUrl(workerId, label) {
          return '/monaco-editor-worker-loader.js'
        }
      }

      window.require.config({
        paths: {
          vs: CDN_URL + '/vs'
        }
      })

      return new Promise((resolve, reject) => {
        window.require(['vs/editor/editor.main'], () => {
          resolve()
        })
  	  });
    },

    async downloadExternalTypeDefs() {
      const packages = [
        {
           name: 'node',
           path: 'node/v8/index.d.ts'
        }
      ]

      const types = []
      for (const { name, path } of packages) {
        const url = `https://raw.githubusercontent.com/DefinitelyTyped/DefinitelyTyped/${DEFINITELY_TYPED_REV}/types/${path}`
        const body = await (await fetch(url)).text()
        types.push({ name, body })
      }

      return types
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

          this.$emit('changed', code)
        })
    },
    async initializeMonacoEditor() {
      const externalTypeDefs = await this.downloadExternalTypeDefs()
      const typeDefs = [
        ...bundledTypeDefs,
        ...externalTypeDefs
      ]

      await this.loadMonacoEditor()
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
  height: 600px; // FIXME: auto grow
}
</style>
