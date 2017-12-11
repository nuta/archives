FactoryBot.define do
  factory :config do
    data_type { "string" }
    sequence(:key) {|n| "key#{n}" }
    value { FFaker::DizzleIpsum.words.join(' ') }

    factory :app_config do
      association :owner, factory: :app
    end

    factory :device_config do
      association :owner, factory: :app
    end
  end
end
