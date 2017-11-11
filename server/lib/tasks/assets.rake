namespace :assets do
  task :precompile do
    system('cd ui && yarn && yarn build')
    raise 'failed to compile assets' if $? != 0

    if ENV['GENERATE_CONFIG_FROM_ENV']
      system('./bin/rails config:generate')
      raise 'failed to generate config files' if $? != 0
    end
  end

  task :clean do
  end
end
