app_dir = File.expand_path("../..", __FILE__)
unicorn_dir = "#{app_dir}/unicorn"
working_directory app_dir

worker_processes 2
preload_app true
timeout 30

listen "#{unicorn_dir}/unicorn.sock", :backlog => 64
pid "#{unicorn_dir}/unicorn.pid"
stderr_path "log/unicorn.stderr.log"
stdout_path "log/unicorn.stdout.log"
