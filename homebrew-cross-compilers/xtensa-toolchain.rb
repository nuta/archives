class XtensaToolchain < Formula
  desc "Toolchain for Xtensa"
  homepage "https://gcc.gnu.org"
  url "http://arduino.esp8266.com/osx-xtensa-lx106-elf-gb404fb9-2.tar.gz"
  sha256 "0cf150193997bd1355e0f49d3d49711730035257bc1aee1eaaad619e56b9e4e6"

  def install
    prefix.install Dir["*"]
    system "rm -rf #{share}/gdb #{bin}/xtensa-lx106-elf-gdb"
  end
end
