const hotClient = require('webpack-hot-middleware/client?noInfo=true&reload=true')

hotClient.subscribe(event => {
  switch (event.action) {
    case 'reload':
      window.location.reload()
      break

    case 'compiling':
      document.body.innerHTML += `compiling...`
      break
  }
})
