FactoryBot.define do
  factory :device do
    app
    user
    sequence(:name) {|n| "device#{n}" }
    device_id        { Digest::SHA256.hexdigest(FFaker::Internet.email)[0, Device::DEVICE_ID_LEN] }
    device_id_prefix { device_id[0, Device::DEVICE_ID_PREFIX_LEN] }
    device_secret    { Digest::SHA256.hexdigest(FFaker::Internet.email)[0, Device::DEVICE_SECRET_LEN] }
    device_type "raspberrypi3"
  end
end
