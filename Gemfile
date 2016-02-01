source 'https://rubygems.org'

gem 'rails', '4.2.5.1'

gem 'puma'
gem 'mysql2', group: :mysql
gem 'pg', group: :postgres

gem 'highline', require: false

group :capistrano do
  gem 'capistrano'
  gem 'capistrano-rails'
  gem 'capistrano-rbenv'
  gem 'capistrano-bundler'
  gem 'capistrano3-puma'
end

group :development do
  gem 'bullet'
  gem 'spring'
  gem 'byebug'
  gem 'awesome_print'
  gem 'hirb'
  gem 'hirb-unicode'
  gem 'guard'
  gem 'guard-rspec'
  gem 'erd', require: false
end

group :test do
  gem 'faker'
  gem 'factory_girl_rails'
  gem 'rspec-rails'
  gem 'coveralls', require: false
  gem 'shoulda-matchers'
end

group :development, :test do
  gem 'sqlite3'
  gem 'pry-rails'
  gem 'pry-doc'
end
