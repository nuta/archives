'use strict'

const functions = require('firebase-functions')
const admin = require('firebase-admin')

admin.initializeApp(functions.config().firebase)

function padStart(str, n, fill) {
  return fill.repeat(n - str.length) + str
}

exports.webhook = functions.https.onRequest((req, res) => {
  let now = new Date()
  let hours = padStart(now.getHours().toString(), 2, '0')
  let mins = padStart(now.getMinutes().toString(), 2, '0')
  let secs = padStart(now.getSeconds().toString(), 2, '0')

  let message = `${hours}:${mins}:${secs}`
  res.send(JSON.stringify({ stores: { message } }))
})
