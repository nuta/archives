FactoryBot.define do
  factory :user do
    sequence(:username) {|n| "user#{n}" }
    email    { "#{username}@example.com" }
    password { FFaker::InternetSE.password }
    confirmed_at { Time.now }
  end

  factory :heavy_user, class: 'User' do
    sequence(:username) {|n| "user#{n}" }
    sequence(:email)    {|n| "email#{n}@example.com" }
    password { FFaker::InternetSE.password }
    confirmed_at { Time.now }

    after(:create) do |user, evaluator|
      app = create(:app, user: user)
      device = create(:device, user: user)
      5.times { create(:config, owner: device) }
      5.times { create(:config, owner: app) }
      5.times { create(:integration, app: app) }
      5.times { create(:deployment, app: app) }
      5.times { create(:app, user: user) }
      5.times { create(:device, user: user) }
    end
  end
end
