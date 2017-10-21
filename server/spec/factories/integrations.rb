FactoryGirl.define do
  factory :integration do
    name { FFaker::InternetSE.slug }
    app
    service "outgoing_webhook"
    config  ({ webhook_url: "http://webhook.url" }.to_json)
  end
end
