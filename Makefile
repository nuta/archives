.PHONY: b bundle s server t test

t test:
	bundle exec rspec

s server:
	bundle exec ./bin/rails server

b bundle:
	bundle install --no-deployment --path vendor/bundle
