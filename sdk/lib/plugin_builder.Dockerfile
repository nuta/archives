FROM ubuntu:16.04
RUN apt-get update -qy && apt-get install -qy curl
RUN curl -sL https://deb.nodesource.com/setup_8.x | bash -
RUN apt-get install -qy zip nodejs build-essential \
  binutils-arm-linux-gnueabihf gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf \
  binutils-aarch64-linux-gnu gcc-aarch64-linux-gnu  g++-aarch64-linux-gnu
RUN npm install -g node-gyp yarn

CMD \
    cp -r /plugin /build && \
    cd /build && \
    if [ -f package.json ]; then \
      rm -rf node_modules && \
      yarn --prod && \
      \
      node-gyp rebuild --arch x64 && \
      mkdir -p native/x64 && \
      mv build/Release/*.node native/x64 && \
      rm -rf build && \
      \
      AR=arm-linux-gnueabihf-ar \
      CC=arm-linux-gnueabihf-gcc \
      CXX=arm-linux-gnueabihf-g++ \
      LINK=arm-linux-gnueabihf-g++ \
      node-gyp rebuild --arch arm && \
      mkdir -p native/arm && \
      mv build/Release/*.node native/arm && \
      rm -rf build && \
      \
      AR=aarch64-linux-gnu-ar \
      CC=aarch64-linux-gnu-gcc \
      CXX=aarch64-linux-gnu-g++ \
      LINK=aarch64-linux-gnu-g++ \
      node-gyp rebuild --arch arm64 && \
      mkdir -p native/arm64 && \
      mv build/Release/*.node native/arm64 && \
      rm -rf build && \
      \
      rm -rf coverage test lib/native node_modules/nan node_modules/.yarn-integrity yarn.lock binding.gyp; \
    fi; \
    rm /dist/plugin.zip; zip -FSr /dist/plugin.zip *
