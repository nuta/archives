if ENV['TRAVIS'] == 'true'
  require 'coveralls'
  Coveralls.wear!
end
