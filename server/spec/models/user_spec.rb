require 'rails_helper'

RSpec.describe User, type: :model do
  describe 'associations' do
    it { is_expected.to has_many(:app).dependent(:destroy) }
    it { is_expected.to has_many(:device).dependent(:destroy) }
  end

  describe 'destroy' do
    subject { create(:user) }
    it_should_behave_like 'a removable model', [App, Device]
  end
end
