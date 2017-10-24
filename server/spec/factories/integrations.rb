FactoryGirl.define do
  factory :integration do
    app
    name { FFaker::InternetSE.slug }
    service "outgoing_webhook"
    config  ({ webhook_url: "http://webhook.url" }.to_json)
  end
end
