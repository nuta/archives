#!/bin/sh
exitcode=0

cd test

RECORD_NAME=test RAILS_ENV=test CODECLIMATE=true bundle exec rails server 2>&1 > log &
PID=$!

sleep 5

bundle exec rails test:run
exitcode=$?

kill -2 $PID
wait $PID

echo "---------------------------------------"
cat log

exit $exitcode
