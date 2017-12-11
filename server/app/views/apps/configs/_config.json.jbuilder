json.extract! config, :key, :data_type, :value, :created_at, :updated_at
json.url app_config_url(config.owner.name, config.key)
