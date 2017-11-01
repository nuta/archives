FactoryBot.define do
  factory :source_file do
    app
    path { SecureRandom.hex(20) + '.js' }
    body { FFaker::Lorem.sentence }
  end
end
