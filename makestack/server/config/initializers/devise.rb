Devise.setup do |config|
  config.authentication_keys = [:username]
  config.mailer_sender = MakeStack.settings[:mailer_sender]
end
