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

  <div id="editor" ref="editor"></div>
</dashboard-layout>
</template>

<script>
import api from "js/api";
import ActionButton from "components/action-button";
import DashboardLayout from "layouts/dashboard";

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
      sampleCode: "loop(3 /* seconds, () => {\n  print('Hello, World!');\n});",
      autosaveAfter: 1000,
      deployButton: "waiting",
      saveButton: "waiting"
    };
  },
  methods: {
    deploy() {
      let comment = "Deployment at " + (new Date()).toString(); // TODO
      this.deployButton = "Deploying...";
      api.deploy(this.appName, ace.edit("editor").getValue(), "", comment, null).then(r => {
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
#editor {
  width: 100%;
  min-height: 300px;
  height: 70vh;
  font-size: 14px;
  border: 7px solid #f3f3f3;
  border-radius: 5px;
}

.save-button {
  width: 120px;
}
</style>
