FactoryBot.define do
  factory :app do
    user
    sequence(:name) {|n| "app#{n}" }
    api  "nodejs"
    os_version "a"
  end
end
