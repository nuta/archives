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
    propxml { <<EOS
  <?xml version="1.0" encoding="UTF-8"?>
  <A:prop xmlns:A="DAV:">
      <A:displayname/>
      <D:calendar-color xmlns:D="http://apple.com/ns/ical/" symbolic-color="purple">
      #123456
      </D:calendar-color>
  </A:prop>
EOS
    }
  end

  factory :schedule do
    type = ""
    summary { generate :todo }
    uri { "#{generate :uuid}.ics" }
    ics { generate :ics }
  end
end
