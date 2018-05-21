{
  "targets": [
    {
      "target_name": "native",
      "sources": [
        "src/module.cpp",
        "src/ioctl.cpp",
        "src/serial.cpp",
        "src/spi.cpp"
      ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")"
      ]
    }
  ]
}
