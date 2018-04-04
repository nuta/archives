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

    context 'valid response with configs' do
      let (:configs) {
        {
          data1: 'hello!',
          data2: 123,
          data3: 10.456,
          data4: false
        }
      }

      let (:config_types) {
        {
          data1: 'string',
          data2: 'integer',
          data3: 'float',
          data4: 'bool'
        }
      }

      it 'updates a device configs' do
        stub_request(:post, config['webhook_url'])
          .to_return(status: 200, body: { configs: configs }.to_json)

        expect {
          CallOutgoingWebhookJob.perform_now(
            url: config['webhook_url'],
            body: "",
            accept_configs: true,
            device: device
          )
        }.not_to raise_exception

        configs.each do |key, value|
          expect(device.configs.where(key: key)).to exist
          expect(device.configs.find_by_key(key).value).to eq(value.to_s)
          expect(device.configs.find_by_key(key).data_type).to eq(config_types[key])
        end
      end
    end

    context 'invalid response' do
      it 'updates a device configs' do
        stub_request(:post, config['webhook_url'])
          .to_return(status: 200, body: {
            configs: "this is not a JSON!"
          }.to_json)

        expect {
          CallOutgoingWebhookJob.perform_now(
            url: config['webhook_url'],
            body: "",
            accept_configs: true,
            device: device
          )
        }.to raise_exception(RuntimeError)

        expect(device.configs.all).to eq([])
      end
    end
  end

  context 'a dead server' do
    it 'does nothing' do
      stub_request(:post, config['webhook_url']).to_timeout
      expect {
        CallOutgoingWebhookJob.perform_now(url: config['webhook_url'], body: "")
      }.to raise_exception(RuntimeError)
    end
  end
end
