namespace :assets do
  task :precompile do
    %x[cd ui && yarn && yarn build]
  end

  task :clean do
  end
end
