require 'rails_helper'

RSpec.describe User, type: :model do
  describe 'associations' do
    it { is_expected.to have_many(:apps).dependent(:destroy) }
    it { is_expected.to have_many(:devices).dependent(:destroy) }
  end

  describe 'destroy' do
    subject { create(:user) }
    it_should_behave_like 'a removable model', [App, Device]
  end
end
