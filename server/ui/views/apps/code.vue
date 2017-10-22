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

    <div id="editor" ref="editor"></div>
  </div>
</dashboard-layout>
</template>

<script>
import api from "js/api";
import ActionButton from "components/action-button";
import DashboardLayout from "layouts/dashboard";
import 'whatwg-fetch';
const Cookies = require('js-cookie')
const JSZip = require('jszip')

function fetchLatestGitHubRelease(url, name) {
  console.log(`==> searching for \`${name}'`)

  return new Promise((resolve, reject) => {
    fetch(`${url}/repos/${name}/releases/latest`).then(resp => {
      return resp.json()
    }).then(json => {
      resolve(json.assets)
    }).catch(reject)
  })
}

async function downloadPlugin(name) {
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

  const assets = await fetchLatestGitHubRelease(githubURL, repo)
  const pluginURL = assets.filter(asset => asset.name.indexOf(name) !== -1)[0].browser_download_url
  if (!pluginURL) {
    throw new Error(`unknown plugin: \`${name}'`)
  }

  console.log(`==> downloading \`${name}'`)
  return (await fetch(pluginURL)).blob()
}

async function mergeZipFiles(pluginName, destZip, srcZip) {
  for (const filepath in srcZip.files) {
    destZip.file(`node_modules/${pluginName}/${filepath}`,
      srcZip.files[filepath].async('arraybuffer'))
  }

  return destZip
}

async function buildApp (code) {
  const appName = 'deploy-test'
  let runtime = 'app-runtime'
  let plugins = [runtime]
  let zip = new JSZip()

  // Populate plugin files.
  for (let i = 0; i < plugins.length; i++) {
    const pluginName = plugins[i]
    const pluginZip = await downloadPlugin(pluginName)
    zip = await mergeZipFiles(pluginName, zip, await (new JSZip()).loadAsync(pluginZip))
  }

  // Copy start.js to the top level.
  const startJsRelPath = `node_modules/${runtime}/start.js`
  if (!zip.files[startJsRelPath]) {
    throw new Error(`runtime start.js not found`)
  }

  zip.file('start.js', zip.files[startJsRelPath].async('arraybuffer'))

  // Copy app files.
  zip.file('main.js', code)
  return new Blob([await zip.generateAsync({ type: 'arraybuffer' })],
                  { type: 'application/zip' })
}



export default {
  components: { ActionButton, DashboardLayout },
  data() {
    return {
      appName: app.$router.currentRoute.params.appName,
      editor: {},
      deployMetadata: {},
      deployMode: "associated",
      targetDeviceName: "",
      devices: [],
      fileName: "",
      prevFileBody: "",
      sampleCode: "loop(3 /* seconds */, () => {\n  print('Hello, World!');\n});",
      autosaveAfter: 1000,
      deployButton: "waiting",
      saveButton: "waiting",
      showUploadNavi: false
    };
  },
  methods: {
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

    async deploy() {
      const code = ace.edit("editor").getValue()
      this.deployButton = "Building...";
      const image = await buildApp(code)

      let comment = "Deployment at " + (new Date()).toString(); // TODO
      this.deployButton = "Deploying...";
      api.deploy(this.appName, image, "", comment, null).then(r => {
        this.deployButton = "done";
        setTimeout(() => { this.deployButton = "waiting"; }, 1500);
      }).catch(error => notify("error", error));
    },
    save() {
      let body = ace.edit("editor").getValue();
      this.saveButton = "doing";
      api.saveFile(this.appName, this.fileName, body).then(r => {
        this.saveButton = "done";
        setTimeout(() => { this.saveButton = "waiting"; }, 1500);
      }).catch(error => notify("error", error));
    }
  },
  beforeMount() {
    api.getFiles(this.appName).then(r => {
      let files = r.json;
      let path, body;

      switch (files.length) {
      case 0:
        path = "app.js";
        body = this.sampleCode;
        break;
      case 1:
        path = files[0].path;
        body = files[0].body;
        break;
      default:
        alert("Editing multiple files is not supported yet.");
      }

      this.fileName = path;
      this.prevFileBody = body;
      let editor = ace.edit("editor");
      editor.setValue(body);
      editor.selection.moveCursorFileStart();
    }).catch(error => notify("error", error));
  },
  mounted() {
    let editor = ace.edit("editor");
    editor.$blockScrolling = Infinity;
    editor.setTheme("ace/theme/solarized_light");
    editor.setShowPrintMargin(false);

    let session = editor.getSession();
    session.setMode("ace/mode/javascript");
    session.setUseSoftTabs(true);

    let component = this;
    let autosaveTimer;
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
  }
};
</script>

<style lang="scss" scoped>
$editor-border-width: 7px;

.editor-wrapper {
  display: block;
  position: relative;

  #editor {
    width: 100%;
    min-height: 300px;
    height: 70vh;
    font-size: 14px;
    border: $editor-border-width solid #f3f3f3;
    border-radius: 5px;
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
