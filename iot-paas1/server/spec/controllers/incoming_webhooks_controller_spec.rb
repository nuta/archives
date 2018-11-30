require 'rails_helper'

RSpec.describe IncomingWebhooksController, type: :controller do
  describe '#invoke' do
    def invoke(command, token, params)
      @request.headers['Content-Type'] = 'application/json'
      post 'invoke',
        params: { token: token },
        body: ({ command: command }.merge(params)).to_json
    end

    let(:user) { create(:user) }
    let(:app) { create(:app, user: user) }
    let(:device) { create(:device, app: app, user: user) }
    let(:integration) { create(:integration, app: app) }
    let(:new_configs) { { foo: 'hello', bar: 123 } }
    let(:data_types) { { foo: 'string', bar: 'integer' } }

    context 'update_app_configs' do
      it 'updates app configs' do
        invoke :update_app_configs, integration.token, { configs: new_configs }
        new_configs.each do |key, value|
          expect(app.configs.find_by_key(key).value).to eq(value.to_s)
          expect(app.configs.find_by_key(key).data_type).to eq(data_types[key])
        end
      end
    end

    context 'update_device_configs' do
      it 'updates device configs' do
        invoke :update_device_configs, integration.token, { device: device.name, configs: new_configs }
        new_configs.each do |key, value|
          expect(device.configs.find_by_key(key).value).to eq(value.to_s)
          expect(device.configs.find_by_key(key).data_type).to eq(data_types[key])
        end
      end
    end

    context 'invalid token' do
      it 'returns :unauthorized' do
        integration2 =  build_stubbed(:integration)
        invoke :update_app_configs, integration2.token, { configs: new_configs }
        new_configs.each do |key, value|
          expect(app.configs.find_by_key(key)).to eq(nil)
        end
      end
    end

    context 'unknown command' do
      it 'returns :bad_request' do
        expect {
          invoke :unknown_command, integration.token, {}
        }.to raise_error(ActionController::BadRequest)
      end
    end
  end
end
