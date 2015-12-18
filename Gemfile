source 'https://rubygems.org'

gem 'rails', '4.2.5'

gem 'mysql2', group: :mysql
gem 'pg', group: :postgres

gem 'backup'
gem 'dotenv'
gem 'highline'

group :production do
  gem 'unicorn'
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
  gem 'fuubar'
end

group :development, :test do
  gem 'sqlite3'
  gem 'pry-rails'
  gem 'pry-doc'
end
