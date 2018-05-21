require 'rails_helper'

RSpec.describe Integration, type: :model do
  describe 'associations' do
    it { is_expected.to belong_to(:app) }
  end

  describe 'validation' do
    it 'does not allow invalid config' do
      config = { service: 'outgoing_webhook', webhook_url: 'file:///etc/passwd' }
      integration = build_stubbed(:integration, config: config)
      expect(integration).not_to be_valid
    end

    it 'does not allow an invalid Slack URL' do
      config = { service: 'slack', webhook_url: 'http://foo.dev' }
      integration = build_stubbed(:integration, config: config)
      expect(integration).not_to be_valid
    end

    it 'does not allow a too long comment' do
      integration = build_stubbed(:integration, comment: 'x' * 257)
      expect(integration).not_to be_valid
    end

    it 'does not allow a invalid JSON' do
      integration = build_stubbed(:integration, config: '{foo:}')
      expect(integration).not_to be_valid
    end

    it 'does not allow a too long config' do
      integration = build_stubbed(:integration, config: 'x' * 257)
      expect(integration).not_to be_valid
    end
  end
end
