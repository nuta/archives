namespace :assets do
  task :precompile do
    system('cd ui && yarn && yarn build')
  end

  task :clean do
  end
end
