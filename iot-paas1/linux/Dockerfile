#
#  How to use:
#
#    docker run \
#      -e TARGET=$TARGET \
#	     -e MAKEFLAGS="-j$(getconf _NPROCESSORS_ONLN)" \
#	     -e BUILD=$BUILD \
#	     -e VERSION=$VERSION \
#	     -v $(realpath ..):/makestack -it makestack/linux
#
FROM ubuntu:16.04
RUN apt-get update -qy && apt-get install -qy curl && \
  curl -sL https://deb.nodesource.com/setup_8.x | bash - && \
  curl -sS https://dl.yarnpkg.com/debian/pubkey.gpg | apt-key add - && \
  echo "deb https://dl.yarnpkg.com/debian/ stable main" | tee /etc/apt/sources.list.d/yarn.list  && \
  apt-get update -qy && \
  apt-get install -qy nodejs build-essential bc dosfstools cpio yarn \
    binutils-arm-linux-gnueabihf gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf \
    binutils-aarch64-linux-gnu gcc-aarch64-linux-gnu  g++-aarch64-linux-gnu \
    pkg-config bison flex unzip

RUN yarn config set cache-folder /makestack/linux/build/downloads/yarn-cache

# Fill yarn cache.
ADD pkgbuilder/package.json /tmp/package.json
RUN cd /tmp && yarn

CMD cd /makestack/linux && make && cp build/$TARGET/*.img /dist
