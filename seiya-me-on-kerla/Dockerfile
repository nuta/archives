#
#  Busybox
#
FROM ubuntu:20.04 as busybox
RUN apt-get update && apt-get install -qy build-essential curl sed
RUN apt-get install -qy musl-tools
RUN curl -fsSL --output tarball.tar.bz2 "https://busybox.net/downloads/busybox-1.31.1.tar.bz2"
RUN mkdir /build && tar xf tarball.tar.bz2 --strip-components=1 -C /build
WORKDIR /build
ADD infra/busybox.config /build/.config
RUN yes n | make oldconfig \
    && ln -s /usr/bin/ar /usr/bin/musl-ar \
    && ln -s /usr/bin/strip /usr/bin/musl-strip \
    && make -j$(nproc)

#
#  http-server
#
FROM ubuntu:20.04 as http-server
RUN apt-get update && apt-get install -qy musl-tools make
ADD http-server /http-server
WORKDIR /http-server
RUN make RELEASE=1 CC=musl-gcc

#
#  www
#
FROM node:17-alpine as www
ADD package.json /repo/package.json 
ADD yarn.lock /repo/yarn.lock
ADD . /repo
WORKDIR /repo
RUN yarn
RUN yarn build

#
#  Initramfs
#
FROM scratch
COPY --from=busybox  /build/busybox_unstripped /bin/busybox

RUN ["/bin/busybox", "--install", "-s"]
RUN mkdir -p /dev /tmp /www
RUN ln -s /proc/metrics /www/metrics
RUN echo "kerla is running" > /www/health
COPY --from=http-server /http-server/build/http-server /bin/http-server
COPY --from=www /repo/out /www
CMD ["/bin/http-server", "-p", "80", "-b", "any", "-c", "64", "-d", "/www"]
