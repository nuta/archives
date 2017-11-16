'use strict'

const functions = require('firebase-functions')
const admin = require('firebase-admin')
const fetch = require('node-fetch')
const moment = require('moment')

const OPEN_WEATHER_MAP_API_KEY = ''
const CITY = 'Tokyo'
const OPEN_WEATHER_MAP_URL = `https://api.openweathermap.org/data/2.5/forecast?q=${CITY}&appid=${OPEN_WEATHER_MAP_API_KEY}`

admin.initializeApp(functions.config().firebase)

exports.webhook = functions.https.onRequest((req, res) => {
  fetch(OPEN_WEATHER_MAP_URL, { headers: { 'Content-Type': 'application/json' } })
    .then(response => response.json()).then(json => {
      const sensorType = req.body.event === 't' ? 'temperature' : 'humidity'
      const sensorData = parseFloat(req.body.body).toFixed(2)
      const weather = json.list[0].weather[0].main.toUpperCase()

      const messages = JSON.stringify([
        moment().format('HH:mm dd  Z'),
        `${sensorType} ${sensorData}`,
        `weather: ${weather}`
      ])

      res.send(JSON.stringify({ stores: { messages } }))
    })
})
