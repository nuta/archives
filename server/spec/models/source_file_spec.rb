require 'rails_helper'

RSpec.describe SourceFile, type: :model do
  describe 'associations' do
    it { is_expected.to belong_to(:app) }
  end

  describe 'validation' do
    it 'does not allow new file beyond the limit of # of files' do
      app = create(:app)
      create_list(:source_file, User::SOURCE_FILES_MAX_NUM_PER_APP, app: app)
      file = build(:source_file, app: app)

      expect(file).not_to be_valid
    end

    it 'does not allow invalid paths' do
      invalid_paths = ['foo/', 'foo/bar.c', '/foo/bar.baz', ' abc', '', '<script']
      invalid_paths.each do |path|
        file = build_stubbed(:source_file, path: path)
        expect(file).not_to be_valid
      end
    end

    it 'does not allow a too long body' do
      file = build_stubbed(:source_file, body: 'x' * (SourceFile::BODY_MAX_LEN + 1))
      expect(file).not_to be_valid
    end
  end
end
