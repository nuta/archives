#!/usr/bin/env node
const fs = require('fs')
const { Esptool } = require("./dist")

async function main() {
  if (!process.argv[3]) {
    console.error("Usage: esptool.js /dev/your-serialport your_program.bin")
    process.exit(1)
  }

  const devicePath = process.argv[2]
  const firmwarePath = process.argv[3]
  const esptool = new Esptool({
    driver: "node",
    path: devicePath,
    baudRate: 115200
  })

  await esptool.open()
  await esptool.flash([
    { address: 0x00001000, data: fs.readFileSync('bootloader.bin') },
    { address: 0x00010000, data: fs.readFileSync(firmwarePath) },
    { address: 0x00008000, data: fs.readFileSync('partitions_singleapp.bin') }
  ])
}

main()
