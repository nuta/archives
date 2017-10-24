require 'rails_helper'

RSpec.describe Deployment, type: :model do
  describe 'associations' do
    it { is_expected.to belong_to(:app) }
  end

  describe 'validations' do
    it 'does not allow invalid image' do
      deployment = build_stubbed(:deployment, image: 'ELF')
      expect(deployment).not_to be_valid
    end

    it 'does not allow too big image' do
      big_image =  'PK' + 'x' * (Deployment::MAX_IMAGE_SIZE)
      deployment = build_stubbed(:deployment, image: big_image)
      expect(deployment).not_to be_valid
    end
  end
end
