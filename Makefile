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
