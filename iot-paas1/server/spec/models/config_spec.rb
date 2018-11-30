require 'rails_helper'

RSpec.describe Config, type: :model do
  describe 'associations' do
    it { is_expected.to belong_to(:owner) }
  end

  describe 'validations' do
    it 'allows valid values' do
      valid_keys = ['hello@example.com', 'foo-bar', 'a' * 30]
      valid_keys.each do |key|
        config = build_stubbed(:app_config, key: key)
        expect(config).to be_valid
      end
    end

    it 'disallows invalid keys' do
      invalid_keys = ['$FOO', '/hoge', nil, 'a' * 70]
      invalid_keys.each do |key|
        config = build_stubbed(:app_config, key: key)
        expect(config).not_to be_valid
      end
    end

    it 'disallows case-sensitive keys' do
      key = 'hello'
      another_one = create(:app_config, key: key)
      config = build_stubbed(:app_config, key: key.upcase, owner_id: another_one.owner_id)
      expect(config).not_to be_valid
    end

    it 'disallows invalid values' do
      invalid_values = [nil, 'a' * 1024]
      invalid_values.each do |value|
        config = build_stubbed(:app_config, value: value)
        expect(config).not_to be_valid
      end
    end

    it 'allows the same key name' do
      user = create(:user)
      config1 = build_stubbed(:app_config, owner: create(:app, user: user))
      config2 = build_stubbed(:device_config, owner: create(:device, user: user))
      expect(config1).to be_valid
      expect(config2).to be_valid
    end
  end
end
