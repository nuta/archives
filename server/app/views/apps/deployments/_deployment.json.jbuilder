json.extract! deployment, :version, :tag, :comment, :deployed_from, :created_at, :updated_at
json.url app_deployment_url(deployment.app.name, deployment.version)
