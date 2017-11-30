FROM ubuntu:16.04
RUN apt-get update -qy && apt-get install -qy curl
RUN curl -sL https://deb.nodesource.com/setup_8.x | bash -
RUN apt-get install -qy zip nodejs build-essential \
  binutils-arm-linux-gnueabihf gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf \
  binutils-aarch64-linux-gnu gcc-aarch64-linux-gnu  g++-aarch64-linux-gnu

RUN npm install -g node-gyp yarn

# Fill yarn cache.
RUN cd /tmp && yarn add nan typescript ts-node mocha istanbul sinon power-assert
ADD ./plugin_builder.sh /plugin_builder.sh
CMD /plugin_builder.sh
