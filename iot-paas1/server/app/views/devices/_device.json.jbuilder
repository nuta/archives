json.extract! device, :name, :device_id, :device_secret, :tag, :device_type,:sakuraio_module_token, :created_at, :updated_at
json.status device.status.value || 'down'
json.last_heartbeated_at device.last_heartbeated_at.value
json.app device.app.try(:name) || nil
