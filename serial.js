var SerialPort = require('serialport'); var port = new 
SerialPort('/dev/tty.usbserial-DN01J4KX'); port.on('open', function() {
  port.write('main screen turn on', function(err) {
    if (err) {
      return console.log('Error on write: ', err.message);
    }
    console.log('message written');
  });
});
// open errors will be emitted as an error event port.on('error', 
port.on('error', function(err) {
  console.log('Error: ', err.message);
})
