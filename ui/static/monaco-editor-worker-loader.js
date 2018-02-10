// https://github.com/Microsoft/monaco-editor#integrate-cross-domain

const CDN_URL = 
'https://cdnjs.cloudflare.com/ajax/libs/monaco-editor/0.10.1/min'

self.MonacoEnvironment = {
  baseUrl: CDN_URL
};

importScripts(CDN_URL + '/vs/base/worker/workerMain.js');
