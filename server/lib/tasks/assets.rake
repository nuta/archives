namespace :assets do
  task :precompile do
    system('cd ui && yarn && yarn build')
    if ENV['GENERATE_SERVER_CONFIG_IN_ASSETS_PRECOMPILE']
      system('./bin/rails config:generate')
    end
  end

  task :clean do
  end
end
