#!/usr/bin/env perl
#
#
#  パディングを追加
#
#

use strict;
use warnings;


#
#  $ARGV[0]: ディスクイメージのファイルパス
#  $ARGV[1]: ファイルサイズ (Bytes)
#

# 引数がきちんとあるかチェック
if(@ARGV < 2){
  die "too few arguments.";
}

my ($filepath, $size) = @ARGV;

#ファイルがあるかチェック
if(!(-f $filepath)){
  die "No such file: '$filepath'.";
}

#ファイルサイズを取得し、パディングを書き込む
my $realSize = -s $filepath;
my $zero     = 0;

open(FH, ">>$filepath") or  die "File open error.";
binmode(FH);

sysseek(FH, 0, 2);

$size -= $realSize;
print FH pack("x$size");

close(FH);

