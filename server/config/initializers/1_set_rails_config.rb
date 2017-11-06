Rails.application.configure do
  config.host = MakeStack.settings[:host]

  config.action_mailer.default_url_options = {
    host: MakeStack.settings[:host],
    port: MakeStack.settings[:port]
  }
end
