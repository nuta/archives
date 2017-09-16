#!/bin/sh
case $TARGET in
server)
  cd server
  bundle exec rspec
  ;;
ui)
  cd server
  npm run build
  ;;
*)
  echo "Unknown $TARGET"
  exit 1
esac