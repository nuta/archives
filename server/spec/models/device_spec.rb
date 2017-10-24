require 'rails_helper'

RSpec.describe Device, type: :model do
  describe 'associations' do
    it { is_expected.to belong_to(:user) }
    it { is_expected.to belong_to(:app) }
    it { is_expected.to have_many(:device_stores).dependent(:destroy) }
    it { is_expected.to have_many(:device_mappings).dependent(:destroy) }
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
  end
end
