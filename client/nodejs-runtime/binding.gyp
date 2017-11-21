{
  "targets": [
    {
      "target_name": "ioctl",
      "sources": [ "lib/ioctl.cpp" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ]
    },
    {
      "target_name": "serial",
      "sources": [ "lib/serial/serial.cpp" ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}
