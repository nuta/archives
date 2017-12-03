#!/bin/sh
set -ue

run_node_gyp(){
  arch=$1
  cross_compile=$2

  AR=${cross_compile}ar CC=${cross_compile}gcc CXX=${cross_compile}g++ LINK=${cross_compile}g++ \
    node-gyp rebuild --arch $arch

  mkdir -p native/$arch
  mv build/Release/*.node native/$arch
}

cp -r /plugin /build
cd /build
if [ -f package.json ]; then
  yarn --ignore-scripts

  if [ -f tsconfig.json ]; then
    mkdir -p node_modules/@types/makestack
    cp /makestack.d.ts node_modules/@types/makestack/index.d.ts
    yarn transpile
  fi;

  if [ -f binding.gyp ]; then
    run_node_gyp x64   ''
    run_node_gyp arm   arm-linux-gnueabihf-
    run_node_gyp arm64 aarch64-linux-gnu-
  fi
fi;

if [ -f .makestackignore ]; then
  rm -rf $(cat .makestackignore)
fi

rm /dist/plugin.zip
zip -FSr /dist/plugin.zip *
