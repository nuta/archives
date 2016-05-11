class Grub < Formula
  desc "The GRand Unified Bootloader"
  homepage "https://www.gnu.org/software/grub/"

  # because he GRUB developers does not release new version since 2012
  # it uses git repository instead of version 2.00
  head "git://git.savannah.gnu.org/grub.git", :revision => "4fe8e6d4a1279b1840171d8e797d911cd8443333"

  depends_on "gcc"
  depends_on "x86_64-elf-gcc"
  depends_on "x86_64-elf-binutils"
  depends_on "flex"
  depends_on "xorriso"
  depends_on "gawk"
  depends_on "automake"

  def install
    args = []
    args << "CC=gcc-5" # use a genuine gcc
    args << "--target=i386"
    args << "--with-platform=pc"
    args << "--prefix=#{prefix}"
    args << "LEX=#{Formula["flex"].opt_bin/'flex'}"
    args << "TARGET_CC=x86_64-elf-gcc"
    args << "TARGET_OBJCOPY=x86_64-elf-objcopy"
    args << "TARGET_STRIP=x86_64-elf-strip"
    args << "TARGET_NM=x86_64-elf-nm"
    args << "TARGET_RANLIB=x86_64-elf-ranlib"
    args << "--disable-nls"

    system "./autogen.sh"
    system "./configure", *args
    system "make"
    system "make", "install"
  end
end
