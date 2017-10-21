json.extract! source_file, :path, :body, :created_at, :updated_at
json.url app_source_file_url(source_file.app.name, source_file.path)
