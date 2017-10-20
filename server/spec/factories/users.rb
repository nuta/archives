FactoryGirl.define do
  factory :user do
    username do
      FFaker::InternetSE.user_name.gsub(".", "-") +
      Array.new(5).map{|_| Random.rand(10).to_s}.join
    end
    email    { FFaker::Internet.email }
    password { FFaker::InternetSE.password }
    confirmed_at { Time.now }
  end
end