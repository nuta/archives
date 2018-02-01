<template>
  <dashboard-layout :title="appName">
    <ul class="nav nav-tabs" role="tablist">
      <li class="nav-item">
        <a class="nav-link active" data-toggle="tab" role="tab" href="#code">Code</a>
      </li>
      <li class="nav-item">
        <a class="nav-link" data-toggle="tab" role="tab" href="#config">Config</a>
      </li>
    </ul>

    <div class="tab-content main">
      <div class="tab-pane show active" role="tabpanel" id="code">
        <button @click="deploy" class="btn btn-primary">
          <i class="fa fa-rocket" aria-hidden="true"></i>
          Deploy
        </button>
        <div class="editor" ref="editor"></div>
      </div>
      <div class="tab-pane" id="config">
        config!
      </div>
    </div>

    <div class="log">
      <p class="line" v-for="line in log">
        <span class="time">{{ line.time | date("HH:mm") }}</span>
        <span class="device">{{ line.device }}</span>
        <span class="body">{{ line.body }}</span>
      </p>
    </div>
  </dashboard-layout>
</template>

<script>
import api from "~/assets/js/api"
import { date } from "~/assets/js/filters"
import DashboardLayout from "~/components/dashboard-layout"
import bundledTypeDefs from "~/assets/dts.json"
import * as monaco from '@timkendrick/monaco-editor'
import 'whatwg-fetch'
const _ = require('lodash')
const JSZip = require('jszip')

export default {
  components: { DashboardLayout },
  filters: { date },
  head: {
    link: [
      { rel: 'stylesheet', href: 'https://fonts.googleapis.com/css?family=Source+Code+Pro:400,600' },
    ]
  },
  asyncData(context) {
    return {
      appName: context.params.name,
      integrations: [],
      configs: [],
      code: '',
      autosaveAfter: 1000,
      deployButton: "Deploy",
      saveButton: "Save",
      editor: null,
      log: []
    }
  },
  methods: {
    uploadFile(event) {
      const reader = new FileReader()
      reader.onload = (event) => {
        debugger
      }

      reader.readAsText(event.dataTransfer.files[0])
      this.showUploadNavi = false
    },

    async packApp (code) {
      let runtime = 'runtime'
      let zip = new JSZip()

      // Copy app files.
      zip.file('app.js', code)

      const zipImage = await zip.generateAsync({
        type: 'arraybuffer',
        compression: 'DEFLATE',
        compressionOptions: {
          level: 9
        }
      })

      return new Blob([zipImage], { type: 'application/zip' })
    },

    async deploy() {
      this.deployButton = "Building..."
      const image = await this.packApp(this.getEditorBody())

      this.deployButton = "Deploying...";
      let comment = "Deployment at " + (new Date()).toString(); // TODO
      const r = await api.deploy(this.appName, image, "", comment, null)

      this.$Notification.success(`Deplyed version ${r.version}.`)
      this.deployButton = "Deploy";
    },

    async save() {
      let body = this.getEditorBody()

      this.saveButton = "Saving"
      await api.saveFile(this.appName, 'app.js', body)

      this.saveButton = "Saved"
      this.prevEditorBody = body
      setTimeout(() => { this.saveButton = "Save" }, 1500)
    },

    getEditorBody() {
      return this.editor.getModel().getValue()
    },

    setEditorBody(body) {
      this.editor.getModel().setValue(body)
    },

    _initializeMonacoEditor(typeDefs) {
      const cdnUrl = 'https://cdnjs.cloudflare.com/ajax/libs/monaco-editor/0.10.1/min'

      return new Promise((resolve, reject) => {
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

        this.editor = monaco.editor.create(document.querySelector('.editor'), {
          theme: 'vs-light',
          fontFamily: 'Source Code Pro',
          automaticLayout: true,
          autoIndent: true,
  	  		value: this.code,
          language: 'javascript',
          minimap: {
            enabled: false
          }
        });

        this.editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.KEY_S, () => {
          // TODO
        })

        this.editor.addCommand(monaco.KeyMod.CtrlCmd | monaco.KeyCode.KEY_P, () => {
          this.editor.getAction('editor.action.quickCommand').run()
        })

        resolve()
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
        const url = `https://raw.githubusercontent.com/DefinitelyTyped/DefinitelyTyped/ea2dc4dc2831c32fe96aa8bb225a42c7b2c9193a/types/${path}`
        const body = await (await fetch(url)).text()
        types.push({ name, body })
      }

      return types
    },
    async initializeMonacoEditor() {
      const externalTypeDefs = await this.downloadExternalTypeDefs()

      await this._initializeMonacoEditor([
        ...bundledTypeDefs,
        ...externalTypeDefs
      ])
    },

    async fetchNewLines() {
      const lines = (await api.getAppLog(this.appName, this.lastFetchedAt)).lines || []
      this.lastFetchedAt = new Date()
      return lines
    }
  },

  async mounted() {
    this.app = await api.getApp(this.appName)
    this.integrations = await api.getIntegrations(this.appName)
    this.configs = await api.getAppConfigs(this.appName)
    for (const file of await api.getFiles(this.appName)) {
      if (file.path === 'app.js') {
        this.code = file.body
      }
    }

    this.prevEditorBody = this.code

    this.log = await this.fetchNewLines()
    this.logFetchTimer = setInterval(async () => {
      this.log = this.log.concat(await this.fetchNewLines())
    }, 5000)

    await this.initializeMonacoEditor()
    // autosave
/*
    setInterval(() => {
      let body = this.getEditorBody();
      if (body !== this.prevEditorBody) {
        this.save()
        return
      }
    }, 3000)
*/
  },

  beforeDestroy() {
    if (Array.isArray(this.editorExtraLibs)) {
      this.editorExtraLibs.map(lib => lib.dispose())
    }

    if (this.editor) {
      this.editor.dispose()
    }

    clearInterval(this.logFetchTimer)
  }
};
</script>

<style lang="scss" scoped>
.main {
  padding: 10px 0px;
}

.editor {
  border: 1px solid #cacaca;
  height: 300px;
}

.editor-caption {
  display: flex;
  justify-content: space-between;
  text-align: right;
  font-size: 12px;
  color: #535353;
}

.actions {
  text-align: right;
}
</style>
