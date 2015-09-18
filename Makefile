.PHONY: b bundle s server t test at autotest

t test:
	bundle exec rspec

at autotest:
	bundle exec guard


s server:
	bundle exec ./bin/rails server

b bundle:
	bundle install --no-deployment --path vendor/bundle
