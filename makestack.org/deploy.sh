#!/bin/sh
yarn build
rm -rf makestack.github.io
git clone git@github.com:makestack/makestack.github.io
cd makestack.github.io
rm -rf *
cp -r ../htdocs/* .
git add .
git cm -m 'auto deploy'
git push origin master
