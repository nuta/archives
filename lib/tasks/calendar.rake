namespace :calendar do
  desc 'list calendars'
  task :list => :environment do
    Calendar.all.find_each do |cal|
      puts "#{cal.props['displayname']}"
    end
  end
end
