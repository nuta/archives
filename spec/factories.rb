
FactoryGirl.define do
  sequence :name do
    Faker::Name.name
  end

  factory :user do
    name { generate :name }
  end
end
