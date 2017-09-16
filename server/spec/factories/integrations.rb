FactoryGirl.define do
  factory :integration do
    app
    service "outgoing_webhook"
    config  ({ webhook_url: "http://webhook.url" }.to_json)
  end
end
