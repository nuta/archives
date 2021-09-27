#!/bin/sh

TOP=../../..

mkdir -p data/homu
cp -r $TOP/icon.png $TOP/apps data/homu

tar czf control.tar.gz -C control .
tar czf data.tar.gz    -C data    .

ar -r homu-desktop.deb debian-binary control.tar.gz data.tar.gz
rm -rf data/homu control.tar.gz data.tar.gz
