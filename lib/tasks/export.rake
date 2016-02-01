require 'highline'

namespace :export do
  desc 'export as a .ics file'
  task :ics => :environment do
    cli = HighLine.new
    calendar = ENV['calendar'] || cli.ask('calendar: ')
    filepath = ENV['filepath'] || cli.ask('filepath: ')
    ics = ""

    Schedule.where(calendar: Calendar.find_by_name(calendar)).find_each do |sched|
      ics += sched.ics.strip + "\n"
    end

    f = open(filepath, "w")
    f.write(ics)
    f.close
  end
end
