require 'rails_helper'

RSpec.describe App, type: :model do
  describe 'associations' do
    it { is_expected.to belong_to(:user) }
    it { is_expected.to have_many(:deployments).dependent(:destroy) }
    it { is_expected.to have_many(:app_stores).dependent(:destroy) }
    it { is_expected.to have_many(:source_files).dependent(:destroy) }
    it { is_expected.to have_many(:integrations).dependent(:destroy) }
    it { is_expected.to have_many(:devices) }
  end

  describe 'validation' do
    it 'does not allow invalid os version' do
     app = build_stubbed(:app, os_version: 'abc')
     expect(app).not_to be_valid
    end

    it 'does not allow new app beyond the limit' do
      user = create(:user)
      create_list(:app, User::APPS_MAX_NUM, user: user)
      app = build(:app, user: user)
      expect(app).not_to be_valid
    end

    it 'does not allow invalid name' do
      app = build_stubbed(:app, name: 'hello123/')
      expect(app).not_to be_valid
    end

    it 'does not allow reserved app names' do
      App::RESERVED_APP_NAMES.each do |app_name|
        app = build(:app, name: app_name)
        expect(app).not_to be_valid
      end
    end
  end
end
