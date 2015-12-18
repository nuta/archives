.PHONY: b bundle s server t test at autotest backup

t test:
	bundle exec rspec
	for workflow in osx-tests/*; do \
		reattach-to-user-namespace automator $$workflow; \
	done

at autotest:
	bundle exec guard

c console:
	bundle exec ./bin/rails console

s server:
	bundle exec ./bin/rails server

b bundle:
	bundle install

dev: bundle
	cp config/database.yml.example config/database.yml
	

production:
	test -f config/database.yml
	bundle install --without development test mysql
	RAILS_ENV=production bundle exec rake db:migrate
	test ! -f .env && touch .env
	echo -n "SECRET_KEY_BASE=" >> .env
	RAILS_ENV=production bundle exec rake secret >> .env

backup:
	bundle exec backup perform -t db_backup --root-path $$PWD/backup
