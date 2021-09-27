#!/usr/bin/env perl
#
#
#  ELFの実行ファイル を rawバイナリファイル に変換する
#
#

use strict;
use warnings;

#
#  $ARGV[0]:  ELFの実行ファイル
#

`strip -O binary  ${ARGV[0]}`;

