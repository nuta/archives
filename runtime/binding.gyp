{
  "targets": [
    {
      "target_name": "ioctl",
      "sources": [ "src/ioctl.cpp" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ]
    },
    {
      "target_name": "serial",
      "sources": [ "src/serial.cpp" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ]
    },
    {
      "target_name": "spi",
      "sources": [ "src/spi.cpp" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}
