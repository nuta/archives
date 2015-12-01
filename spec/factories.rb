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

  sequence :ics do
      <<-EOS
BEGIN:VCALENDAR
END:VCALENDAR
      EOS
  end

  factory :calendar do
    name { generate :name }
  end

  factory :schedule do
    type = ""
    summary { generate :todo }
    uri { "/calendar/1/#{generate :uuid}.ics" }
    ics { generate :ics }
  end
end
