class ArmElfEabiBinutils < Formula
  desc "GNU binutils for ARM"
  homepage "http://www.gnu.org/software/binutils/"
  url "http://ftpmirror.gnu.org/binutils/binutils-2.26.tar.bz2"
  sha256 "c2ace41809542f5237afc7e3b8f32bb92bc7bc53c6232a84463c423b0714ecd9"

  def install
    args = []
    args << "--program-prefix=arm-elf-eabi-"
    args << "--target=arm-elf-eabi"
    args << "--prefix=#{prefix}"
    args << "--infodir=#{info}"
    args << "--mandir=#{man}"
    args << "--disable-nls"
    args << "--disable-debug"
    args << "--disable-dependency-tracking"
    args << "--disable-werror"

    mkdir "build" do
      system "../configure", *args
      system "make"
      system "make", "install"
    end

    info.rmtree
  end
end
