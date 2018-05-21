FactoryBot.define do
  factory :integration do
    app
    sequence(:name) {|n| "integration#{n}" }
    service "outgoing_webhook"
    config  ({ webhook_url: "http://webhook.url" }.to_json)
  end
end
