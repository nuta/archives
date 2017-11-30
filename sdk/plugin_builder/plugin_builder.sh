#!/bin/sh
set -ue

run_node_gyp(){
  arch=$1
  cross_compile=$1

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
    yarn transpile
  fi;

  if [ -f binding.gyp ]; then
    run_node_gyp x64
    run_node_gyp arm arm-linux-gnueabihf-
    run_node_gyp arm64 aarch64-linux-gnu-
  fi

  npm prune --production --loglevel=warn
fi;

rm -rf \
  coverage test lib/native node_modules/nan node_modules/.yarn-integrity \
  yarn.lock package-lock.json binding.gyp build

if [ -f .makestackignore ]; then
  rm -rf $(cat .makestackignore)
fi

rm /dist/plugin.zip
zip -FSr /dist/plugin.zip *
