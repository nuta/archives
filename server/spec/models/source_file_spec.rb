require 'rails_helper'

RSpec.describe SourceFile, type: :model do
  describe 'associations' do
    it { is_expected.to belong_to(:app) }
  end

  describe 'validation' do
    it 'does not allow new file beyond the limit' do
      app = create(:app)
      create_list(:source_file, User::SOURCE_FILES_MAX_NUM_PER_APP, app: app)
      file = build(:source_file, app: app)

      expect(file).not_to be_valid
    end
  end
end
