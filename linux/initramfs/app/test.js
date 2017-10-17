#!/bin/node
let fs = require('fs')
let https = require('https')

console.log('*** userland testing tool')

let fsTest = new Promise((resolve, reject) => {
  fs.readFile('/etc/hosts', 'utf-8', (err, data) => {
    return err ? reject(err) : resolve(data)
  })
})

let netTest = new Promise((resolve, reject) => {
  https.get('https://httpbin.org/ip', r => {
    r.on('data', (json) => {
      resolve({ status: r.statusCode, ip: JSON.parse(json).origin })
    })
  }).on('error', e => reject(e))
})

Promise.all([fsTest, netTest]).then(r => {
  console.log(JSON.stringify(r, null, 4))
  console.log('*** success!')
}).catch(e => {
  console.error(e)
  console.log('*** fail!')
})
