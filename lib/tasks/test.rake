namespace :test do
  
  desc 'run tests'
  task :run do
    system "bundle exec ./test/replay.rb test/records/*"
  end
end
