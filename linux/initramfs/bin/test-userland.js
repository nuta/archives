#!/bin/node
let fs = require('fs');
let https = require('https');

function main() {
  console.log('*** userland testing tool');

  console.log('==> file system');
  console.log(fs.readFileSync('/etc/hosts', 'utf-8'));
    
  console.log('==> networking');
  https.get('https://httpbin.org/ip', r => {
    console.log('status:', r.statusCode);
     
    r.on('data', (json) => {
      console.log('ip:', JSON.parse(json).origin);
    });
  });
}

try {
  main();
  console.error('*** success!');
} catch(e) {
  console.error(e);
  console.error('*** fail!');
}
