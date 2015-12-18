working_directory File.expand_path("../..", __FILE__)

worker_processes 2
preload_app true
timeout 30

listen "/var/run/busybook.sock", :backlog => 64
pid "/var/run/busybook.pid"
stderr_path "log/unicorn.stderr.log"
stdout_path "log/unicorn.stdout.log"
