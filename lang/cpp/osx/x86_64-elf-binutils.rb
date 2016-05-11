class X8664ElfBinutils < Formula
  desc "GNU binutils for i386 & x86_64 (ELF/EFI PE)"
  homepage "http://www.gnu.org/software/binutils/"
  url "http://ftpmirror.gnu.org/binutils/binutils-2.26.tar.bz2"
  sha256 "c2ace41809542f5237afc7e3b8f32bb92bc7bc53c6232a84463c423b0714ecd9"

  def install
    args = []
    args << "--program-prefix=x86_64-elf-"
    args << "--enable-targets=x86_64-elf,i386-elf"
    args << "--target=x86_64-elf"
    args << "--prefix=#{prefix}"
    args << "--infodir=#{info}"
    args << "--mandir=#{man}"
    args << "--disable-nls"
    args << "--disable-debug"
    args << "--disable-dependency-tracking"
    args << "--disable-werror"
    args << "--enable-64-bit-bfd"

    mkdir "build" do
      system "../configure", *args
      system "make"
      system "make", "install"
    end

    info.rmtree
  end
end
