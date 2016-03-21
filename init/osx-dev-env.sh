#!/bin/sh
set -eu

cd "$(git rev-parse --show-toplevel)"
brew install rbenv

rbenv install --skip-existing
eval "$(rbenv init -)"

if ! bundle help 2>&1 > /dev/null; then
    gem install bundler
fi

bundle install --path vendor/bundle --jobs 4 --without mysql postgres

cp config/database.yml.example config/database.yml
bundle exec rake db:migrate
bundle exec rake user:add username=guest password=123
