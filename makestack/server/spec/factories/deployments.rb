FactoryBot.define do
  factory :deployment do
    app
    image { "PK#{FFaker::InternetSE.slug}" }
    image_shasum { Digest::SHA256.hexdigest(image) }
    debug "debug body"
    deployed_from "client"
  end
end
