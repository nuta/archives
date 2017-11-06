require 'rails_helper'

RSpec.describe App, type: :model do
  describe 'associations' do
    it { is_expected.to belong_to(:user) }
    it { is_expected.to have_many(:deployments).dependent(:destroy) }
    it { is_expected.to have_many(:stores).dependent(:destroy) }
    it { is_expected.to have_many(:source_files).dependent(:destroy) }
    it { is_expected.to have_many(:integrations).dependent(:destroy) }
    it { is_expected.to have_many(:devices) }
  end

  describe 'destroy' do
    subject { create(:app) }
    it_should_behave_like 'a removable model', [Store, SourceFile, Integration]
  end

  describe '#disassociate_devices' do
    let!(:app) { create(:app) }
    let!(:devices) { create_list(:device, 10, app: app, user: app.user) }

    it 'nullifies devices' do
      expect {
        app.destroy
      }.to change(Device, :count).by(0)

      devices.each do |device|
        expect(Device.where(id: device)).to be_exists
        device.reload
        expect(device.app).to be_nil
      end
    end
  end

  describe 'validations' do
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

    it 'does not allow invalid names' do
      invalid_names = ['hello123/', '"foo"', nil, 'x' * 256, '0abc', '-asd', 'd<']
      invalid_names.each do |name|
        app = build_stubbed(:app, name: name)
        expect(app).not_to be_valid
      end
    end

    it 'does not allow reserved app names' do
      App::RESERVED_APP_NAMES.each do |app_name|
        app = build(:app, name: app_name)
        expect(app).not_to be_valid
      end
    end

    it 'does not allow invalid apis' do
      invalid_apis = ['1', nil, 'x' * 256, 'abc', 'linux2', 'makestack_']
      invalid_apis.each do |api|
        app = build_stubbed(:app, api: api)
        expect(app).not_to be_valid
      end
    end
  end

  describe 'default_value_for :os_version' do
    it 'set the latest os version if it is not provided' do
      latest = MakeStack.os_releases.keys[-1]
      app = build_stubbed(:app, os_version: nil)
      expect(app).to be_valid
      expect(app.os_version).to eq(latest)
    end
  end
end
