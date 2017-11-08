FactoryBot.define do
  factory :deployment do
    app
    image "PK"
    debug "debug body"
    deployed_from "client"
  end
end
