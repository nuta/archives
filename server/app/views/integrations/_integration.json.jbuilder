json.extract! integration, :name, :service, :config, :comment, :token, :created_at, :updated_at
json.url app_integration_url(integration.app.name, integration.name)
