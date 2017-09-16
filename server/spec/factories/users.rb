FactoryGirl.define do
  factory :user do
    username { FFaker::InternetSE.user_name.gsub(".", "-") }
    email    { FFaker::Internet.email }
    password { FFaker::InternetSE.password }
    confirmed_at { Time.now }
  end
end