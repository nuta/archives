FactoryGirl.define do
  sequence :name do
    FFaker::Name.name
  end

  factory :user do
    user { generate :name }
  end
end
