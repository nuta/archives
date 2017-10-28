<template>
<dashboard-layout title="Code">
  <section>
    <div class="two-columns">
      <div class="columns">
        <action-button v-on:click="deploy" positive
         :state="deployButton" waiting-message="Deploy" doing-message="Deploying..."
         done-message="Deployed" icon="fa-rocket"></action-button>
        <action-button v-on:click="save" :state="saveButton" waiting-message="Save"
         doing-message="Saving..." done-message="Saved" icon="fa-upload"></action-button>
      </div>

      <div>
        <div class="rows">
          <div class="right-aligned columns">
            <a href="/documentation">
              <button>
                <i class="fa fa-book" aria-hidden="true"></i>
                Documentation
              </button>
            </a>
          </div>
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

    <modal title="Add a new file" :active="showNewFileModal" @close="showNewFileModal = false">
      <input type="text" v-model="newFileName" placeholder="Filename">
      <button @click="addNewFile">Add a new file</button>
    </modal>

    <div class="tabs">
      <div v-for="file in files" class="tab" @click="editing = file.id" :class="{ active: editing === file.id }">{{ file.path }}</div>
      <div class="tab" @click="showNewFileModal = true">
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
</dashboard-layout>
</template>

<script>
import api from "js/api";
import ActionButton from "components/action-button";
import Modal from "components/modal";
import DashboardLayout from "layouts/dashboard";
import 'whatwg-fetch';
const Cookies = require('js-cookie')
const JSZip = require('jszip')

export default {
  components: { ActionButton, DashboardLayout, Modal },
  data() {
    return {
      appName: app.$router.currentRoute.params.appName,
      devices: [],
      files: [],
      autosaveAfter: 1000,
      deployButton: "waiting",
      saveButton: "waiting",
      showUploadNavi: false,
      editing: '',
      output: '',
      showOutputPanel: true,
      showNewFileModal: false,
      newFileName: ''
    };
  },
  methods: {
    addNewFile() {
      const id = `editor${this.files.length}`
      const path = this.newFileName

      this.files.push({
        id,
        path,
        body: '',
        editing: false
      })

      this.showNewFileModal = false
      this.newFileName = ''

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
    async downloadPlugin(name) {
      let githubURL

      if (window.location.host === 'localhost:8080' && Cookies.get('use_local_mock_github_server')) {
        // Use local mock github releases server (tools/github-releases-server.py).
        githubURL = `http://localhost:8080`
      } else {
        githubURL = `https://api.github.com`
      }

      let repo
      if (name.match(/^[a-zA-Z0-9\-\_]+$/)) {
        // Builtin plugins (e.g. app-runtime)
        repo = 'seiyanuta/makestack'
      } else if (name.match(/^[a-zA-Z0-9\-\_]+\/[a-zA-Z0-9\-\_]+$/)) {
        // Plugins on GitHub (e.g. octocat/temperature-sensor)
        repo = name
      } else {
        throw new Error(`invalid plugin name: \`${name}'`)
      }

      const assets = await this.fetchLatestGitHubRelease(githubURL, repo)
      const pluginURL = assets.filter(asset => asset.name.indexOf(name) !== -1)[0].browser_download_url
      if (!pluginURL) {
        throw new Error(`unknown plugin: \`${name}'`)
      }

      return (await fetch(pluginURL)).blob()
    },
    async mergeZipFiles(pluginName, destZip, srcZip) {
      for (const filepath in srcZip.files) {
        destZip.file(`node_modules/${pluginName}/${filepath}`,
          srcZip.files[filepath].async('arraybuffer'))
      }

      return destZip
    },
    async buildApp (files) {
      let runtime = 'app-runtime'
      let plugins = [runtime]
      let zip = new JSZip()

      // Populate plugin files.
      for (const pluginName of plugins) {
        this.logOutput(`Downloading a plugin \`${pluginName}'`)
        const pluginZip = await this.downloadPlugin(pluginName)
        zip = await this.mergeZipFiles(pluginName, zip, await (new JSZip()).loadAsync(pluginZip))
      }

      // Copy start.js to the top level.
      const startJsRelPath = `node_modules/${runtime}/start.js`
      if (!zip.files[startJsRelPath]) {
        throw new Error(`runtime start.js not found`)
      }

      this.logOutput(`Copying start.js from \`${runtime}'`)
      zip.file('start.js', zip.files[startJsRelPath].async('arraybuffer'))

      // Copy app files.
      for (const file of files) {
        this.logOutput(`Adding \`${file.path}'`)
        zip.file(file.path, file.body)
      }

      return new Blob([await zip.generateAsync({ type: 'arraybuffer' })],
                      { type: 'application/zip' })
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
      this.deployButton = "Building..."

      this.logOutput('Building the app...')
      const image = await this.buildApp(this.getFiles())

      this.logOutput('Generated a .zip file, deploying...')
      let comment = "Deployment at " + (new Date()).toString(); // TODO
      this.deployButton = "Deploying...";
      api.deploy(this.appName, image, "", comment, null).then(r => {
        const took = (((new Date()) - timeStarted) / 1000).toPrecision(3)
        this.deployButton = "done"
        this.logOutput(`Deployed #${r.json.version}, took ${took} seconds`)
        setTimeout(() => { this.deployButton = "waiting"; }, 1500);
      }).catch(error => notify("error", error));
    },
    save() {
      for (const file of this.files) {
        let body = ace.edit(file.id).getValue();
        this.saveButton = "doing";
        api.saveFile(this.appName, file.path, body).then(r => {
          this.saveButton = "done";
          setTimeout(() => { this.saveButton = "waiting"; }, 1500);
        }).catch(error => notify("error", error));
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
      editor.setTheme("ace/theme/solarized_light")
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
  beforeMount() {
    api.getFiles(this.appName).then(r => {
      let files = []
      for (const file of r.json) {
        files.push({
          id: `editor${i}`,
          path: file.path,
          body: file.body,
          editing: i == 0
        })
      }

      this.files = files
      this.editing = this.files[0].id
      this.$nextTick(() => {
        for (const file of this.files) {
          this.addEditor(file.id, file.path, file.body)
        }
      })
    })
//    }).catch(error => notify("error", error));
  },
  mounted() {
    let component = this;
    let autosaveTimer;
    /*
    session.on('change', function(e) {
      if (component.prevFileBody == editor.getValue())
        return;

      clearTimeout(autosaveTimer);
      autosaveTimer = setTimeout(() => {
        let body = editor.getValue();
        if (body == component.prevFileBody)
          return;

        component.prevFileBody = body;
        component.save();
      }, component.autosaveAfter);
    });
    */
  }
};
</script>

<style lang="scss" scoped>
$editor-border-width: 3px;
$editor-border-color: #e6e6ec;

.editor-wrapper {
  display: block;
  min-height: 300px;
  position: relative;

  .tabs {
    .tab {
      display: inline-block;
      border: 2px solid $editor-border-color;
      padding: 10px;
      font-size: 15px;

      &.active {
        background: #ffcc00;
      }

      &:hover {
        cursor: pointer;
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
