FactoryBot.define do
  factory :store do
    data_type { "string" }
    key   { FFaker::InternetSE.slug }
    value { FFaker::DizzleIpsum.words.join(' ') }

    factory :app_store do
      association :owner, factory: :app
    end

    factory :device_store do
      association :owner, factory: :app
    end
  end
end
