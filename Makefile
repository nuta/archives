.PHONY: b bundle s server
b bundle:
	bundle install

s server:
	bundle exec ./bin/rails server

t test:
	bundle exec rake test
