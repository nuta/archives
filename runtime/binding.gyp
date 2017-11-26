{
  "targets": [
    {
      "target_name": "ioctl",
      "sources": [ "lib/native/ioctl.cpp" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ]
    },
    {
      "target_name": "serial",
      "sources": [ "lib/native/serial.cpp" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ]
    },
    {
      "target_name": "spi",
      "sources": [ "lib/native/spi.cpp" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}
