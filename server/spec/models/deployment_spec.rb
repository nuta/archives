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

    it 'does not allow too debug data' do
      debug =  'x' * (Deployment::MAX_DEBUG_DATA_SIZE + 1)
      deployment = build_stubbed(:deployment, debug: debug)
      expect(deployment).not_to be_valid
    end

    it 'does not allow too long comments' do
      deployment = build_stubbed(:deployment, comment: 'a' * 1025)
      expect(deployment).not_to be_valid
    end

    it 'does not allow invalid tags' do
      invalid_tags = ['"foo-bar-baz', '00asd', 'a' * 129]
      invalid_tags.each do |tag|
        deployment = build_stubbed(:deployment, tag: tag)
        expect(deployment).not_to be_valid
      end
    end
  end
end
