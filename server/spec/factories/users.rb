FactoryBot.define do
  factory :user do
    username { 'a' + SecureRandom.hex(13) }
    email    { "#{username}@example.com" }
    password { FFaker::InternetSE.password }
    confirmed_at { Time.now }
  end

  factory :heavy_user, class: 'User' do
    username { 'a' + SecureRandom.hex(13) }
    email    { "#{username}@example.com" }
    password { FFaker::InternetSE.password }
    confirmed_at { Time.now }

    after(:create) do |user, evaluator|
      app = create(:app, user: user)
      device = create(:device, user: user)
      5.times { create(:store, owner: device) }
      5.times { create(:store, owner: app) }
      5.times { create(:integration, app: app) }
      5.times { create(:deployment, app: app) }
      5.times { create(:source_file, app: app) }
      5.times { create(:app, user: user) }
      5.times { create(:device, user: user) }
    end
  end
end
