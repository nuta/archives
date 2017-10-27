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
      let (:stores) {
        {
          data1: 'hello!',
          data2: 123,
          data3: 10.456,
          data4: false
        }
      }

      let (:store_types) {
        {
          data1: 'string',
          data2: 'integer',
          data3: 'float',
          data4: 'bool'
        }
      }

      it 'updates a device stores' do
        stub_request(:post, config['webhook_url'])
          .to_return(status: 200, body: { stores: stores }.to_json)

        expect {
          CallOutgoingWebhookJob.perform_now(
            url: config['webhook_url'],
            body: "",
            accept_stores: true,
            device: device
          )
        }.not_to raise_exception

        stores.each do |key, value|
          expect(device.stores.where(key: key)).to exist
          expect(device.stores.find_by_key(key).value).to eq(value.to_s)
          expect(device.stores.find_by_key(key).data_type).to eq(store_types[key])
        end
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

        expect(device.stores.all).to eq([])
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
