FactoryBot.define do
  factory :deployment do
    app
    image "PK"
    image_shasum { Digest::SHA256.hexdigest('PK') }
    debug "debug body"
    deployed_from "client"
  end
end
