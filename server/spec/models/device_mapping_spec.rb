require 'rails_helper'

RSpec.describe DeviceMapping, type: :model do
  describe 'associations' do
    it { is_expected.to belong_to(:device) }
  end

  describe 'validations' do
    it 'disallows same tokens' do
      mapping1 = create(:device_mapping, token: 'a')
      mapping2 = build_stubbed(:device_mapping, token: 'a')

      expect(mapping1).to be_valid
      expect(mapping2).not_to be_valid
    end
  end
end
