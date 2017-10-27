require 'rails_helper'

RSpec.describe Store, type: :model do
  describe 'associations' do
    it { is_expected.to belong_to(:owner) }
  end

  describe 'validations' do
    it 'allows valid values' do
      valid_keys = ['hello@example.com', 'foo-bar', 'a' * 30]
      valid_keys.each do |key|
        store = build_stubbed(:app_store, key: key)
        expect(store).to be_valid
      end
    end

    it 'disallow invalid keys' do
      invalid_keys = ['$FOO', '/hoge', nil, 'a' * 70]
      invalid_keys.each do |key|
        store = build_stubbed(:app_store, key: key)
        expect(store).not_to be_valid
      end
    end

    it 'disallow invalid values' do
      invalid_values = [nil, 'a' * 1024]
      invalid_values.each do |value|
        store = build_stubbed(:app_store, value: value)
        expect(store).not_to be_valid
      end
    end

    it 'allows the same key name' do
      user = create(:user)
      store1 = build_stubbed(:app_store, owner: create(:app, user: user))
      store2 = build_stubbed(:device_store, owner: create(:device, user: user))
      expect(store1).to be_valid
      expect(store2).to be_valid
    end
  end
end
