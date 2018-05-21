class KludgeForRails5
  def initialize(app, options={})
    @app = app
    @requests = []
  end

  def call(env)
    # https://github.com/rails/rails/issues/24827
    env['CONTENT_TYPE'] ||= 'application/octet-stream'
    @app.call(env)
  end
end

Rails.configuration.middleware.insert(0, KludgeForRails5)
