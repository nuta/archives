namespace :assets do
  task :precompile do
    system('cd ui && yarn && yarn build')
    if ENV['GENERATE_CONFIG_FROM_ENV']
      system('./bin/rails config:generate')
    end
  end

  task :clean do
  end
end
