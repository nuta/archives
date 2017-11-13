FROM ubuntu:16.04
RUN apt-get update -qy && apt-get install -qy curl
RUN curl -sL https://deb.nodesource.com/setup_8.x | bash -
RUN apt-get install -qy nodejs build-essential bc dosfstools \
  binutils-arm-linux-gnueabihf gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf \
  binutils-aarch64-linux-gnu gcc-aarch64-linux-gnu  g++-aarch64-linux-gnu
RUN npm install -g node-gyp yarn

RUN mkdir /makestack
ADD build/client /makestack/client
ADD pkgbuilder /makestack/linux/pkgbuilder
ADD packages /makestack/linux/packages

WORKDIR /makestack/linux
RUN cd pkgbuilder && yarn

CMD ./pkgbuilder/pkgbuilder $TARGET && cp build/$TARGET/*.img /dist
