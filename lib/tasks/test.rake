namespace :test do

  desc 'run tests'
  task :run do
    error = false
    Dir['test/records/*'].each do |file|
      puts ""
      puts "********************************************"
      puts "    #{file}"
      puts "********************************************"

      system "bundle exec ./test/replay.rb #{file}"

      if !error and $?.exitstatus != 0
        error = true
      end
    end

    abort "ERROR: some tests failed" if error
  end
end
