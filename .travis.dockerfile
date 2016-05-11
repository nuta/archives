FROM ubuntu:16.04
RUN apt-get update && apt-get install -y \
    python3-pip make git sudo

ADD . /resea
WORKDIR /resea
RUN cd sdk && python3 setup.py install

CMD ./.travis.sh
