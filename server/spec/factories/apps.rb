FactoryGirl.define do
  factory :app do
    user
    api  "linux"
    os_version "a"
    name { FFaker::InternetSE.slug.gsub(".", "-") }
  end
end
