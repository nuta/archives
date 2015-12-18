require 'yaml'

#
# $ backup perform -t db_backup
#
Backup::Model.new(:db_backup, 'dump database') do
  split_into_chunks_of 250

  database_yml = YAML.load(open("config/database.yml").read)
  production = database_yml["production"]
  if production["adapter"] != "postgresql"
    abort "use PostgreSQL in 'production'"
  end

  database PostgreSQL do |db|
    db.name     = production["database"]
    db.username = production["username"]
    db.password = production["password"]
    db.host     = production["host"]
    db.port     = production["port"]
  end

  store_with Local do |local|
    local.path = "db/backup/"
    local.keep = 7
  end

  compress_with Custom do |compressor|
    compressor.command   = 'xz -3'
    compressor.extension = '.xz'
  end
end
