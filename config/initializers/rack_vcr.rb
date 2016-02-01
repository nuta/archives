if Rails.env.test? or Rails.development?
    Rails.configuration.middleware.insert(0, Rack::VCR)
end
