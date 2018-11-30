FROM ubuntu:16.04

ADD . /resea
WORKDIR /resea

RUN apt-get update && apt-get install -qy sudo
RUN ./tools/install-deps

CMD ./ci/run
