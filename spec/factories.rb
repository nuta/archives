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
    uri { generate :uuid }
    propxml { <<EOS
      {
        "displayname": "My Work",
        "calendar-color": "#123456"
      }
EOS
    }
  end

  factory :schedule do
    type = ""
    summary { generate :todo }
    uri { "#{generate :uuid}.ics" }
    ics { generate :ics }
    association :calendar, factory: :calendar
  end
end
