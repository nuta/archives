json.extract! store, :key, :data_type, :value, :created_at, :updated_at
json.url device_store_url(store.owner.name, store.key)
