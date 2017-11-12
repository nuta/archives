<template>
<app-layout path="code" :app-name="appName">
  <remote-content :loading="loading"></remote-content>
  <div>
    <section>
      <div class="uk-grid">
        <div>
          <label class="uk-form-label">Edit</label>
          <div class="uk-from-controls">
            <button v-on:click="saveAll" class="uk-button uk-button-small uk-button-default">
              <i class="fa fa-upload" aria-hidden="true"></i>
              {{ saveButton }}
            </button>

            <button v-on:click="undo" class="uk-button uk-button-small uk-button-default">
              <i class="fa fa-undo" aria-hidden="true"></i>
            </button>

            <button v-on:click="redo" class="uk-button uk-button-small uk-button-default">
              <i class="fa fa-repeat" aria-hidden="true"></i>
            </button>

            <button v-on:click="search" class="uk-button uk-button-small uk-button-default">
              <i class="fa fa-binoculars" aria-hidden="true"></i>
            </button>
          </div>
        </div>

        <div>
          <label class="uk-form-label">Deploy</label>
          <div class="uk-from-controls">
            <select class="uk-select uk-form-small uk-form-width-medium">
              <option>associated devices</option>
            </select>

            <button v-on:click="deploy" class="uk-button uk-button-small uk-button-primary">
              <i class="fa fa-rocket" aria-hidden="true"></i>
              {{ deployButton }}
            </button>
          </div>
        </div>

        <div>
          <label class="uk-form-label">Panels</label>
          <div class="uk-from-controls">
            <button v-on:click="showOutput" class="uk-button uk-button-small uk-button-default">
              <i class="fa fa-align-left" aria-hidden="true"></i>
              Output
            </button>

            <button v-on:click="showLog" class="uk-button uk-button-small uk-button-default">
              <i class="fa fa-list" aria-hidden="true"></i>
              Log
            </button>
          </div>
        </div>

        <div>
          <label class="uk-form-label">Misc.</label>
          <div class="uk-from-controls">
            <a href="/documentation">
              <button class="uk-button uk-button-small uk-button-default">
                <i class="fa fa-question-circle" aria-hidden="true"></i>
              </button>
            </a>
          </div>
        </div>
      </div>
    </section>

    <div class="editor-wrapper" @dragover.prevent="showUploadNavi = true" >
      <div class="upload-navi" v-show="showUploadNavi" @dragleave.prevent="showUploadNavi = false" @drop.prevent="uploadFile">
        <i class="fa fa-lg fa-cloud-upload" aria-hidden="true"></i>
        &nbsp;
        Drag & drop your source file here!
      </div>

      <div class="uk-flex-top" uk-modal id="add-new-file-modal">
        <div class="uk-modal-dialog">
          <div class="uk-modal-header">
            <button class="uk-modal-close-default" type="button" uk-close></button>
            <h2 class="uk-modal-title">Add a new file</h2>
          </div>

          <form @submit.prevent="addNewFile">
            <div class="uk-modal-body">
                <input type="text" class="uk-input" v-model="newFileName" placeholder="Filename">
            </div>
            <div class="uk-modal-footer">
              <input type="submit" class="uk-button uk-button-primary" value="Add a new file">
            </div>
          </form>
        </div>
      </div>

      <div class="tabs">
        <div v-for="file in files" class="tab" @click="editing = file.id" :class="{ active: editing === file.id }">{{ file.path }}</div>
        <div class="tab new-file-button" uk-toggle="target: #add-new-file-modal">
          <i class="fa fa-plus" aria-hidden="true"></i>
        </div>
      </div>

      <div v-for="file in files" class="editors">
        <div class="editor" v-show="editing == file.id" :id="file.id" :ref="file.id"></div>
      </div>

      <div v-if="showOutputPanel" class="output-panel">
        <div class="header">
          <span class="title">Deploy Log</span>
          <span class="close" @click="showOutputPanel = false">
            <i class="fa fa-window-close" aria-hidden="true"></i>
          </span>
        </div>

        <div class="body">{{ output }}</div>
      </div>
    </div>
  </div>
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
      files: [],
      prevFileContents: {},
      autosaveAfter: 1000,
      deployButton: "Deploy",
      saveButton: "Save",
      showUploadNavi: false,
      editing: '',
      output: '',
      showOutputPanel: true,
      newFileName: ''
    };
  },
  methods: {
    undo() {

    },

    redo() {

    },

    search() {

    },

    showOutput() {

    },

    showLog() {

    },

    addNewFile() {
      UIkit.modal(document.querySelector('#add-new-file-modal')).hide()

      const id = `editor${this.files.length}`
      const path = this.newFileName
      const body = ''

      this.files.push({
        id,
        path,
        body,
        editing: false
      })

      this.newFileName = ''
      this.prevFileContents[path] = body

      this.$nextTick(() => {
        this.addEditor(id, path, '')
        this.editing = id
      })
    },
    uploadFile(event) {
      const reader = new FileReader()
      reader.onload = (event) => {
        const editor = ace.edit("editor")
        editor.setValue(event.target.result)
        editor.selection.moveCursorFileStart()
      }

      reader.readAsText(event.dataTransfer.files[0])
      this.showUploadNavi = false
    },
    clearOutput() {
      this.output = ""
    },
    logOutput(line) {
      this.output += `${line}\n`
    },
    fetchLatestGitHubRelease(url, name) {
      return new Promise((resolve, reject) => {
        fetch(`${url}/repos/${name}/releases/latest`).then(resp => {
          return resp.json()
        }).then(json => {
          resolve(json.assets)
        }).catch(reject)
      })
    },
    loadAppYAML() {
      const appYAML = this.files.filter(file => file.path === 'app.yaml')[0]
      return (appYAML) ?  yaml.safeLoad(appYAML.body) : {}
    },
    async downloadAndExtractPackage(name, zip, basepath) {
      this.logOutput(`Downloading \`${name}'`)
      const pluginZip = await api.downloadPlugin(name)

      this.logOutput(`Extracting \`${name}'`)
      zip = await this.mergeZipFiles(basepath, zip, await (new JSZip()).loadAsync(pluginZip))
      return zip
    },
    async mergeZipFiles(basepath, destZip, srcZip) {
      for (const filepath in srcZip.files) {
        destZip.file(`${basepath}/${filepath}`,
          srcZip.files[filepath].async('arraybuffer'))
      }

      return destZip
    },
    async buildApp (files) {
      let runtime = 'nodejs-runtime'
      let plugins = this.loadAppYAML().plugins || []
      plugins = plugins.map(pluginName => `nodejs-${pluginName}`)
      let zip = new JSZip()

      // Download the runtime.
      zip = await this.downloadAndExtractPackage(runtime, zip, `node_modules/${runtime}`)

      // Populate plugin files.
      for (const pluginName of plugins) {
        zip = await this.downloadAndExtractPackage(pluginName, zip, `plugins/${pluginName}`)
        if (!zip.files[`plugins/${pluginName}/package.json`]) {
          zip.file(`plugins/${pluginName}/package.json`,
            JSON.stringify({ name: pluginName, private: true }))
        }
      }

      // Copy start.js to the top level.
      this.logOutput(`Copying start.js from \`${runtime}'`)
      const startJsRelPath = `node_modules/${runtime}/start.js`
      zip.file('start.js', zip.files[startJsRelPath].async('arraybuffer'))

      // Copy app files.
      for (const file of files) {
        this.logOutput(`Adding \`${file.path}'`)
        zip.file(file.path, file.body)
      }

      const zipImage = await zip.generateAsync({
        type: 'arraybuffer',
        compression: 'DEFLATE',
        compressionOptions: {
          level: 9
        }
      })

      return new Blob([zipImage], { type: 'application/zip' })
    },
    getFiles() {
      let files = []
      for (const file of this.files) {
        files.push({
          path: file.path,
          body: ace.edit(file.id).getValue()
        })
      }

      return files
    },
    async deploy() {
      const timeStarted = new Date()
      this.clearOutput()
      this.deployButton = "Building..."

      this.logOutput('Building the app...')
      const image = await this.buildApp(this.getFiles())

      this.logOutput('Generated a .zip file, deploying...')
      let comment = "Deployment at " + (new Date()).toString(); // TODO
      this.deployButton = "Deploying...";

      const r = await api.deploy(this.appName, image, "", comment, null)
      const took = (((new Date()) - timeStarted) / 1000).toPrecision(3)
      this.deployButton = "Done!"
      this.logOutput(`Deployed #${r.version}, took ${took} seconds`)
      setTimeout(() => { this.deployButton = "Deploy"; }, 1500);
    },
    async saveAll() {
      for (const file of this.files) {
        let body = ace.edit(file.id).getValue()
        this.saveButton = "Saving"
        await api.saveFile(this.appName, file.path, body)
        this.prevFileContents[file.path] = body
        this.saveButton = "Done"
        setTimeout(() => { this.saveButton = "Save" }, 1500)
      }
    },
    addEditor(id, path, body) {
      const editor = ace.edit(id)
      const ext = path.split('.').pop()
      this.configureAceEditor(editor, ext)
      this.setEditorBody(id, body)
    },
    setEditorBody(id, body) {
      const editor = ace.edit(id)
      editor.setValue(body)
      editor.selection.moveCursorFileStart()
    },
    configureAceEditor(editor, ext) {
      editor.$blockScrolling = Infinity
      editor.setTheme('ace/theme/xcode')
      editor.setShowPrintMargin(false)

      const session = editor.getSession()
      const modes = {
        js: 'javascript',
        yaml: 'yaml'
      }
      session.setMode('ace/mode/' + (modes[ext] || 'text'))
      session.setUseSoftTabs(true)
    }
  },
  async beforeMount() {
    this.files = (await api.getFiles(this.appName)).map((file, i) => {
      this.prevFileContents[file.path] = file.body
      return {
        id: `editor${i}`,
        path: file.path,
        body: file.body,
        editing: i === 0
      }
    })

    this.editing = this.files[0].id
    this.$nextTick(() => {
      for (const file of this.files) {
        this.addEditor(file.id, file.path, file.body)
      }

      this.loading = false
      this.$Progress.finish()
    })
  },
  mounted() {
    // autosave
    setInterval(() => {
      for (const file of this.files) {
        let body = ace.edit(file.id).getValue()
        if (body !== this.prevFileContents[file.path]) {
          this.saveAll()
          return
        }
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

  .tabs {
    .tab {
      background: #f3f3f3;
      display: inline-block;
      border: solid $editor-border-color;
      border-width: 1px 1px 0px 1px;
      font-size: 15px;
      padding: 5px 10px;
      opacity: 0.7;
      vertical-align: bottom;
      margin-right: 2px;
      box-sizing: border-box;

      &.active {
        background: #ffffff;
        opacity: 1;
      }

      &:hover {
        cursor: pointer;
      }

      &.new-file-button {
        background: #ffffff;
        padding: 5px 7px;
      }
    }
  }

  .editor {
    width: 100%;
    box-sizing: border-box;
    min-height: 300px;
    height: 40vh;
    font-size: 14px;
    border: $editor-border-width solid $editor-border-color;
  }

  .output-panel {
    color: #fcfcfa;
    background: #2c292d;
    height: 200px;
    font-family: "Source Code Pro", monospace;
    font-size: 13px;
    overflow: scroll;

    .header {
      background: #cfcfcf;
      color: #2c292d;
      padding: 5px 5px 5px 10px;
      display: flex;
      justify-content: space-between;

      .close:hover {
        cursor: pointer;
      }
    }

    .body {
      padding: 7px;
      white-space: pre;
    }
  }
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
  width: 120px;
}
</style>
