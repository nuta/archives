require 'securerandom'

FactoryGirl.define do
  sequence :name do
    Faker::Name.name
  end

  sequence :todo do
    "Meet #{Faker::Name.name}"
  end

  sequence :uuid do
      SecureRandom.uuid
  end

  factory :user do
    name { generate :name }
  end

  factory :schedule do
    type = ""
    summary { generate :todo }
    uri { generate :uuid }
  end
end
