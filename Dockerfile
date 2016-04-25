FROM ruby:2.3
MAINTAINER Seiya Nuta <nuta@seiya.me>

RUN mkdir -p /app
ADD . /app
WORKDIR /app
RUN bundle install --deployment --without development --jobs 2
