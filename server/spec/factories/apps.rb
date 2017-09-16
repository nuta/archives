FactoryGirl.define do
  factory :app do
    user
    api  "linux"
    name { FFaker::InternetSE.slug.gsub(".", "-") }
  end
end
