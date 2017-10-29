require 'rails_helper'

RSpec.describe Device, type: :model do
  describe 'associations' do
    it { is_expected.to belong_to(:user) }
    it { is_expected.to belong_to(:app) }
    it { is_expected.to have_many(:stores).dependent(:destroy) }
    it { is_expected.to have_many(:device_mappings).dependent(:destroy) }
  end

  describe 'destroy' do
    subject { create(:device) }
    it_should_behave_like 'a removable model', [Store, DeviceMapping]
  end

  describe 'validations' do
    it 'does not allow reserved device names' do
      Device::RESERVED_DEVICE_NAMES.each do |device_name|
        device = build_stubbed(:device, name: device_name)
        expect(device).not_to be_valid
      end
    end

    it 'does not allow new device beyond the limit' do
      user = create(:user)
      create_list(:device, User::DEVICES_MAX_NUM, user: user)
      device = build(:device, user: user)
      expect(device).not_to be_valid
    end

    it 'does not allow invalid names' do
      invalid_names = ['hello123/', '"foo"', nil, 'x' * 256, '0abc', '-asd', 'd<']
      invalid_names.each do |name|
        device = build_stubbed(:device, name: name)
        expect(device).not_to be_valid
      end
    end

    it 'does not allow invalid tags' do
      invalid_tags = ['"foo-bar-baz', '00asd', 'a' * 129]
      invalid_tags.each do |tag|
        device = build_stubbed(:device, tag: tag)
        expect(device).not_to be_valid
      end
    end
  end

  describe '#device_id_prefix_is_prefix' do
    it 'does not allow invalid device_id' do
      device1 = build_stubbed(:device, device_id: nil,
                              device_id_prefix: 'a' * 40)
      device2 = build_stubbed(:device, device_id: 'tooshort')

      expect(device1).not_to be_valid
      expect(device2).not_to be_valid
    end

    it 'does not allow invalid device_id_prefix' do
      device1 = build_stubbed(:device, device_id_prefix: nil)
      device2 = build_stubbed(:device, device_id_prefix: '1' + 'a' * 39,
                              device_secret: 'a' * 40)

      expect(device1).not_to be_valid
      expect(device2).not_to be_valid
    end

    it 'does not allow invalid device_secret' do
      device1 = build_stubbed(:device, device_secret: nil)
      device2 = build_stubbed(:device, device_secret: 'tooshort')

      expect(device1).not_to be_valid
      expect(device2).not_to be_valid
    end
  end
end
