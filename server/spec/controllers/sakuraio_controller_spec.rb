require 'rails_helper'

RSpec.describe SakuraioController, type: :controller do
  describe '#webhook' do
    let!(:token) { 'abcdEF123xyz' }
    let(:message) {
      JSON.parse(<<~EOS
      {
        "module": "#{token}",
        "type": "channels",
        "datetime": "2017-10-30T01:50:10.407404451Z",
        "payload": {
          "channels": [
            {
                "channel": 0,
                "type": "b",
                "value": "87a13101a23130c0",
                "datetime": "2017-10-30T01:50:10.151406408Z"
            },
            {
                "channel": 1,
                "type": "b",
                "value": "a2313102a23132a0",
                "datetime": "2017-10-30T01:50:10.288406408Z"
            },
            {
                "channel": 2,
                "type": "b",
                "value": "a23136a161a23137",
                "datetime": "2017-10-30T01:50:10.306406408Z"
            },
            {
                "channel": 3,
                "type": "b",
                "value": "00a3323535ccff00",
                "datetime": "2017-10-30T01:50:10.339406408Z"
            }
          ]
        }
      }
EOS
  )}

    context 'device exists' do
      let!(:user) { create(:user) }
      let!(:app) { create(:app, user: user) }
      let!(:deployment) { create(:deployment, app: app) }
      let!(:device) { create(:device, user: user, app: app) }
      let!(:device_mapping) {
        create(:device_mapping, device: device, token: token, token_type: 'sakuraio')
      }
      let!(:integration) {
        create(:integration, app: app, service: 'sakuraio',
          config: '{"incoming_webhook_token": "abc"}')
      }

      it 'handles a message correctly' do
        stub_request(:post, "https://api.sakura.io/incoming/v1/").to_return(status: 200)

        post :webhook, params: message
        expect(device.status.value).to eq('ready')
        expect(device.last_heartbeated_at.value).not_to eq(nil)
      end
    end
  end
end
