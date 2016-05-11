FROM ubuntu:16.04
RUN apt-get update && apt-get install -y \
    python3-pip make git sudo

RUN cd sdk && python3 setup.py install
ADD .travis.sh /tmp/travis.sh

CMD /tmp/travis.sh
