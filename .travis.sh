#!/bin/sh
exitcode=0

cd test
bundle exec rake db:migrate
bundle exec rake user:add name=guest password=123
bundle exec rails server 3>&1 > log &
sleep 5
PID=$$

for record in records/*; do
    bundle exec ./replay-record.rb $record
    if [ $? != 0 ]; then
      exitcode=1
    fi
done

echo "---------------------------------------"
cat log

exit $exitcode
