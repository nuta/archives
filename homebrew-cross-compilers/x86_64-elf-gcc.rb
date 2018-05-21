class X8664ElfGcc < Formula
  desc "GNU compiler collection for i386-elf & x86_64-elf"
  homepage "https://gcc.gnu.org"
  url "http://ftpmirror.gnu.org/gcc/gcc-6.1.0/gcc-6.1.0.tar.bz2"
  sha256 "09c4c85cabebb971b1de732a0219609f93fc0af5f86f6e437fd8d7f832f1a351"

  depends_on "gmp"
  depends_on "isl"
  depends_on "libmpc"
  depends_on "mpfr"
  depends_on "x86_64-elf-binutils"

  def install
    args = []
    args << "--program-prefix=x86_64-elf-"
    args << "--enable-languages=c,c++"
    args << "--enable-targets=x86_64-elf,i386-elf"
    args << "--target=x86_64-elf"
    args << "--prefix=#{prefix}"
    args << "--disable-nls"
    args << "--without-headers"
    args << "--with-gmp=#{Formula["gmp"].opt_prefix}"
    args << "--with-mpfr=#{Formula["mpfr"].opt_prefix}"
    args << "--with-mpc=#{Formula["libmpc"].opt_prefix}"
    args << "--with-ld=#{Formula["x86_64-elf-binutils"].opt_bin/'x86_64-elf-ld'}"
    args << "--with-as=#{Formula["x86_64-elf-binutils"].opt_bin/'x86_64-elf-as'}"

    mkdir "build" do
      system "../configure", *args
      system "make", "all-gcc"
      system "make", "all-target-libgcc"
      system "make", "install-gcc"
      system "make", "install-target-libgcc"
    end

    info.rmtree
    man7.rmtree
  end
end
