.PHONY: b bundle s server t test at autotest

t test:
	bundle exec rspec

at autotest:
	bundle exec guard

c console:
	bundle exec ./bin/rails console

s server:
	bundle exec ./bin/rails server

b bundle:
	bundle install
	bundle --binstubs

dev: bundle
	cp config/database.yml.example config/database.yml
	

production:
	test -f config/database.yml
	bundle install --without development test mysql
	RAILS_ENV=production bundle exec rake db:migrate
	echo -n "SECRET_KEY_BASE=" > .env
	RAILS_ENV=production bundle exec rake secret >> .env
