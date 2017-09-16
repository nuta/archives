FactoryGirl.define do
  factory :device_store do
    device
    data_type { "string" }
    key   { FFaker::InternetSE.slug }
    value { FFaker::DizzleIpsum.words }
  end
end
