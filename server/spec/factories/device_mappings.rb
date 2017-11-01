FactoryBot.define do
  factory :device_mapping do
    device
    token_type 'sakuraio'
    token  { Digest::SHA256.hexdigest(FFaker::Internet.email) }
  end
end
