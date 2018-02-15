namespace :assets do
  task :precompile do
    if ENV['GENERATE_CONFIG_FROM_ENV']
      system('./bin/rails config:generate')
      raise 'failed to generate config files' if $? != 0
    end

    ENV['PLATFORM'] = 'web'
    system(ENV, 'cd ui && yarn && yarn generate && cp -r dist/* ../public')
    raise 'failed to compile assets' if $? != 0
  end

  task :clean do
  end
end
