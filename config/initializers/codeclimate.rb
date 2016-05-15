if ENV['CODECLIMATE'] == 'true'
  require "codeclimate-test-reporter"
  CodeClimate::TestReporter.start
end
