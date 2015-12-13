namespace :user do
  desc 'add an user'
  task :add => :environment do
    cli = HighLine.new
    name = ENV['name']     || cli.ask('username: ')
    pass = ENV['password'] || cli.ask('password: ') {|q| q.echo = false }
    User.create(name: name, password: pass)
    Calendar.create(name: 'Private', propxml: '', uri: 'private')
  end

  desc 'delete an user'
  task :delete => :environment do
    cli = HighLine.new
    name = ENV['name'] || cli.ask('username: ')
    User.find_by_name!(name).delete

    # TODO: delete her/his calendars
  end

  desc 'list users'
  task :list => :environment do
    User.all.each do |u|
      puts "#{u.name}"
    end
  end
end
