require 'yaml'

namespace :config do
  task :generate do
    config_json = {
      RECAPTCHA_SITEKEY: ENV.fetch('RECAPTCHA_SITEKEY'),
      ROUTER_MODE: ENV.fetch('ROUTER_MODE', 'hash')
    }.to_json

    settings_yml = {
      protocol: ENV.fetch('SERVER_PROTOCOL'),
      host: ENV.fetch('SERVER_HOST'),
      port: ENV.fetch('SERVER_PORT'),
      mailer_sender: ENV.fetch('MAILER_SENDER', "bot@#{ENV.fetch('SERVER_HOST')}")
    }.stringify_keys.to_yaml

    open('ui/config.json', 'w').write(config_json)
    open('config/settings.production.yml', 'w').write(settings_yml)

    puts "generated ui/config.json and config/settings.production.yml!"
  end
end
