#!/bin/sh
exitcode=0

cd test

cat << EOF > config/database.yml
sqlite: &sqlite
  adapter: sqlite3
  database: db/test.sqlite3

mysql: &mysql
  adapter: mysql
  username: root

postgresql: &postgresql
  adapter: postgresql
  username: postgres

test:
  pool: 5
  timeout: 5000
  host: localhost
  password:
  database: busybook_test
  <<: *<%= ENV['DB'] %>

EOF

RAILS_ENV=test COVERALLS=true bundle exec rails server 3>&1 > log &
PID=$!

sleep 5

bundle exec rake test:run
exitcode=$?

kill -2 $PID
wait $PID

echo "---------------------------------------"
cat log

exit $exitcode
