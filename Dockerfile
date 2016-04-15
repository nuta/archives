FROM ruby:2.3
MAINTAINER Seiya Nuta <nuta@seiya.me>

RUN mkdir -p /app
ADD . /app
ADD database.yml /app/config/database.yml

WORKDIR /app
RUN bundle install

CMD rails s -b 0.0.0.0
