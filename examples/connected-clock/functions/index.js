'use strict'

const functions = require('firebase-functions')
const admin = require('firebase-admin')

admin.initializeApp(functions.config().firebase)

function padStart(str, n, fill) {
  return fill.repeat(n - str.length) + str
}

exports.webhook = functions.https.onRequest((req, res) => {
  const sensorData = Math.floor(req.body.body)
  const event = req.body.event
  const now = new Date()
  const hours = padStart(now.getHours().toString(), 2, '0')
  const mins = padStart(now.getMinutes().toString(), 2, '0')

  let messages = JSON.stringify([
    `${hours}:${mins}`,
    `${event === 't' ? 'temperature' : 'humidity'} ${sensorData}`,
    `weather: i duuno`
  ])

  res.send(JSON.stringify({ stores: { messages } }))
})
