require 'rails_helper'

RSpec.describe CallOutgoingWebhookJob, type: :job do
  let(:app) { create(:app) }
  let(:integration) { create(:integration, app: app) }
  let(:config) { JSON.parse(integration.config) }
  let(:device) { create(:device, app: app) }

  context 'a alive server' do
    it 'invokes a webhook' do
      stub_request(:post, config['webhook_url'])
        .to_return(status: 200, body: "helloooo")

      expect {
        CallOutgoingWebhookJob.perform_now(url: config['webhook_url'], body: "")
      }.not_to raise_exception
    end

    context 'valid response with stores' do
      it 'updates a device stores' do
        stub_request(:post, config['webhook_url'])
          .to_return(status: 200, body: {
            stores: { message: 'hello!?' }
          }.to_json)

        expect {
          CallOutgoingWebhookJob.perform_now(
            url: config['webhook_url'],
            body: "",
            accept_stores: true,
            device: device
          )
        }.not_to raise_exception

        expect(device.device_stores.where(key: 'message')).to exist
      end
    end

    context 'invalid response' do
      it 'updates a device stores' do
        stub_request(:post, config['webhook_url'])
          .to_return(status: 200, body: {
            stores: "this is not a JSON!"
          }.to_json)

        expect {
          CallOutgoingWebhookJob.perform_now(
            url: config['webhook_url'],
            body: "",
            accept_stores: true,
            device: device
          )
        }.not_to raise_exception

        expect(device.device_stores.all).to eq([])
      end
    end
  end

  context 'a dead server' do
    it 'does nothing' do
      stub_request(:post, config['webhook_url']).to_timeout
      expect {
        CallOutgoingWebhookJob.perform_now(url: config['webhook_url'], body: "")
      }.not_to raise_exception
    end
  end
end
