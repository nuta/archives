FactoryBot.define do
  factory :app do
    user
    api  "nodejs"
    os_version "a"
    name { FFaker::InternetSE.slug.gsub(".", "-") }
  end
end
