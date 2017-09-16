#!/bin/sh

case $TARGET in
server)
  cd server
  bundle install --jobs 2 --path vendor/bundle
  bundle exec rails db:migrate RAILS_ENV=test
  ;;
ui)
  cd server
  npm install -g yarn
  yarn
  ;;
*)
  echo "Unknown $TARGET"
  exit 1
esac