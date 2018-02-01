<template>
  <app-layout path="code" :app-name="appName">
    <remote-content :loading="loading">
      <div slot="content">
        <section>
          <div class="uk-grid">
            <div>
              <div class="uk-from-controls">
                <button v-on:click="save" class="save-button uk-button uk-button-small uk-button-default">
                  <i class="fa fa-upload" aria-hidden="true"></i>
                  {{ saveButton }}
                </button>
              </div>
            </div>

            <div>
              <div class="uk-from-controls">
                <button v-on:click="deploy" class="deploy-button uk-button uk-button-small uk-button-primary">
                  <i class="fa fa-rocket" aria-hidden="true"></i>
                  {{ deployButton }}
                </button>
              </div>
            </div>
          </div>
        </section>

        <div class="editor-wrapper" @dragover.prevent="showUploadNavi = true" >
          <div class="editor" ref="editor"></div>
          <div class="upload-navi" v-show="showUploadNavi" @dragleave.prevent="showUploadNavi = false" @drop.prevent="uploadFile">
            <i class="fa fa-lg fa-cloud-upload" aria-hidden="true"></i>
            &nbsp;
            Drag & drop your source file here!
          </div>
        </div>
      </div>
    </remote-content>
  </app-layout>
</template>

<script>
import api from "js/api"
import RemoteContent from "components/remote-content"
import AppLayout from "layouts/app"
import 'whatwg-fetch'
const Cookies = require('js-cookie')
const JSZip = require('jszip')
const yaml = require('js-yaml')

export default {
  components: { AppLayout, RemoteContent },
  data() {
    return {
      appName: app.$router.currentRoute.params.appName,
      loading: true,
      devices: [],
      code: '',
      autosaveAfter: 1000,
      deployButton: "Deploy",
      saveButton: "Save",
      showUploadNavi: false,
      editor: null
    };
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
      const startJs = "require('./app')"

      // Copy app files.
      zip.file('app.js', code)
      zip.file('start.js', startJs)

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

    initializeMonacoEditor() {
      return new Promise((resolve, reject) => {
        window.require.config({ paths: { 'vs': '/monaco-editor/vs' }});
    	  window.require(['vs/editor/editor.main'], () => {
          monaco.languages.typescript.typescriptDefaults.setCompilerOptions({
            allowJs: true,
            target: monaco.languages.typescript.ScriptTarget.ES2017,
            moduleResolution: monaco.languages.typescript.ModuleResolutionKind.NodeJs,
            module: monaco.languages.typescript.ModuleKind.CommonJS,
            typeRoots: ["node_modules/@types"]
          });

          this.editor = monaco.editor.create(this.$refs.editor, {
            theme: 'vs-dark',
            automaticLayout: true,
            autoIndent: true,
    	  		value: this.code,
    	  		language: 'javascript'
          });

          resolve()
    	  });
      })
    }
  },

  async beforeMount() {
    const appJsTemplate = [
      "const { Timer, println } = require('makestack')",
      "",
      "Timer.interval(1, () => {",
      "  println('Hello!')",
      "})"
    ].join('\n')

    const remoteFiles = await api.getFiles(this.appName)
    this.code = remoteFiles['app.js'] || appJsTemplate
    this.prevEditorBody = this.code

    await this.initializeMonacoEditor()
    this.loading = false
    this.$Progress.finish()
  },

  mounted() {
    // autosave
    setInterval(() => {
      let body = this.getEditorBody();
      if (body !== this.prevEditorBody) {
        this.save()
        return
      }
    }, 3000)
  }
};
</script>

<style lang="scss" scoped>
$editor-border-width: 1px;
$editor-border-color: #a6a6ac;

.editor-wrapper {
  display: block;
  min-height: 300px;
  margin-top: 20px;
  position: relative;
}

.editor {
  width: 100%;
  height: 70vh;
  border: 1px solid #cacaca;
}

.upload-navi {
  padding: $editor-border-width;
  padding-top: 30px;
  z-index: 100;
  position: absolute;
  top: 0;
  left: 0;
  background: rgba(0, 0, 0, 0.6);
  color: #fff;
  font-size: 20px;
  font-weight: 600;
  text-align: center;
  width: 100%;
  height: 100%;
}

.save-button {
  width: 100px;
}

.deploy-button {
  width: 140px;
}
</style>
