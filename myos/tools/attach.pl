#!/usr/bin/env perl
#
#
#  ファイルを連結
#
#

use strict;
use warnings;


use Slurp;

my $output;

#
#  $ARGV[0]:  出力先
#  $ARGV[1-]: ファイル
#

# 引数がきちんとあるかチェック
if(@ARGV < 2){
  die "too few arguments.";
}

$output = shift;
open FH, ">$output";


foreach my $file (@ARGV){

  print FH slurp($file);
}

close FH;

