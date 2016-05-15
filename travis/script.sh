#!/bin/sh
exitcode=0

cd test

RAILS_ENV=test CODECLIMATE=true bundle exec rails server 3>&1 > log &
PID=$!

sleep 5

bundle exec rake test:run
exitcode=$?

kill -2 $PID
wait $PID

echo "---------------------------------------"
cat log

exit $exitcode
