
      //
      //  Stage 1: Register the device
      //
      let device = (await api.registerDevice(this.deviceName, this.deviceType)).json
      
            //
            //  Stage 2: Download the OS image
            //
            const basename = path.basename(this.osImageURL)
            const originalDiskImageFile = path.join(process.env.HOME, `.makestack/caches/${basename}`)
            try {
              fs.mkdirSync(path.join(process.env.HOME, '.makestack'))
            } catch (e) {
              // ignore
            }
      
            try {
              fs.mkdirSync(path.join(process.env.HOME, '.makestack/caches'))
            } catch (e) {
              // ignore
            }
      
            fs.writeFileSync(originalDiskImageFile, await (await fetch(this.osImageURL)).buffer())
      
            //
            //  Stage 3: Overwrite config.sh
            //
            const diskImageFile = path.join(os.tmpdir(), uuid() + '.img')
      
            // TODO: What if the image is large?
            let image = fs.readFileSync(originalDiskImageFile)
            image = replaceBuffer(image, device.device_id, 'DEVICE_ID')
            image = replaceBuffer(image, device.device_secret, 'DEVICE_SECRET')
            image = replaceBuffer(image, api.serverURL, 'SERVER_URL_abcdefghijklmnopqrstuvwxyz1234567890')
            image = replaceBuffer(image, this.adapter, 'ADAPTER')
            fs.writeFileSync(diskImageFile, image)
      
            //
            //  Stage 4: Flash!
            //
            const diskImageSize = fs.statSync(diskImageFile).size
      
            // TODO: support Linux (GNU one)
            const argv = ['dd', 'bs=4m', `if=${diskImageFile}`, `of=${this.devFile}`]
            const options = { name: 'MakeStack Installer' }
            console.log(argv)
            let error, stdout, stderr = await sudo.exec(argv.join(' '), options)
      
            console.log(stdout)
            console.error(stderr)
            if (error) {
              this.message = 'something went wrong :('
              alert(error)
              return
            }
      
            this.message = 'done!'