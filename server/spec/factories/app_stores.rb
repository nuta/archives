FactoryGirl.define do
  factory :app_store do
    app
    data_type { "string" }
    key   { FFaker::InternetSE.slug }
    value { FFaker::DizzleIpsum.words.join(' ') }
  end
end
