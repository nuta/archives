FactoryGirl.define do
  factory :device_mapping do
    device
    type   'sakuraio'
    token  { Digest::SHA256.hexdigest(FFaker::Internet.email) }
  end
end
