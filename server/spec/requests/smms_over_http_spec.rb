require "rails_helper"

RSpec.describe "SMMS over HTTP", type: :request do
  def send_msg(device, version: 1, device_info:, app_version: 0, os_version: 'a')
    payload = {
      SMMSService::SMMS_VERSION => version,
      SMMSService::SMMS_DEVICE_INFO => device_info,
      SMMSService::SMMS_OS_VERSION => os_version,
      SMMSService::SMMS_APP_VERSION => app_version,
      SMMSService::SMMS_DEVICE_ID => device.device_id
    }.to_msgpack

    timestamp, hmac = SMMSService::sign(device, payload)

    headers = {
      'Authorization' => "SMMS #{timestamp} #{hmac}",
      'Content-Type' => 'application/octet-stream'
    }

    post '/api/v1/smms', headers: headers, params: payload

    begin
      payload = MessagePack.unpack(response.body)
    rescue
      return {}
    end

    return payload
  end

  describe "heartbeating" do
    context "not associated to any apps" do
      it "returns :ok" do
        device = create(:device, app: nil)
        payload = send_msg(device, device_info: 1) # booting
        expect(response).to have_http_status(:ok)
        expect(payload).to eq({})
      end
    end

    context "associated to a app with no deployments" do
      it "returns :ok" do
        device = create(:device)
        payload = send_msg(device, device_info: 1) # booting
        expect(response).to have_http_status(:ok)
        expect(payload).to eq({})
      end
    end

    context "associated to a app with some deployments" do
      it "returns :ok" do
        app = create(:app)
        device = create(:device, app: app)
        deployments = create_list(:deployment, 10, app: app)

        payload = send_msg(device, device_info: 1) # booting
        expect(response).to have_http_status(:ok)

        expect(payload[SMMSService::SMMS_APP_UPDATE_REQUEST]).to eq(deployments[-1].version)
      end
    end

    context "future version umber" do
      it "returns :forbidden" do
        device = create(:device, app: nil)
        payload = send_msg(device, version: 123456789, device_info: 1)
        expect(response).to have_http_status(:bad_request)
        expect(payload).to eq({})
      end
    end

    context "invalid authetication header" do
      it "returns :forbidden" do
        device = create(:device, app: nil)
        post '/api/v1/smms', headers: { authetication: 'SMMS 0001' }
        expect(response).to have_http_status(:forbidden)
        expect(response.body).to eq('')
      end
    end
  end

  describe 'stores' do
    context "some device stores are set" do
      it "returns stores" do
        device = create(:device)
        stores = create_list(:device_store, 5, device: device)

        payload = send_msg(device, device_info: 1) # booting
        expect(response).to have_http_status(:ok)

        stores.each_with_index do |store, i|
          expect(payload[SMMSService::SMMS_STORE + i]).to eq([store.key, store.value])
        end
      end
    end

    context "some app stores are set" do
      it "returns stores" do
        device = create(:device, app: create(:app))
        stores = create_list(:app_store, 5, app: device.app)

        payload = send_msg(device, device_info: 1) # booting
        expect(response).to have_http_status(:ok)

        stores.each_with_index do |store, i|
          expect(payload[SMMSService::SMMS_STORE + i]).to eq([store.key, store.value])
        end
      end
    end

    context "some device and app stores are set" do
      it "returns stores" do
        device = create(:device, app: create(:app))
        create(:device_store, device: device, key: 'k1', value: 'v1')
        create(:device_store, device: device, key: 'k2', value: 'v2')
        create(:device_store, device: device, key: 'k3', value: 'v3')
        create(:app_store, app: device.app,   key: 'k4', value: 'v5')
        create(:app_store, app: device.app,   key: 'k5', value: 'v5')
        create(:app_store, app: device.app,   key: 'k1', value: 'v6')

        payload = send_msg(device, device_info: 1) # booting
        expect(response).to have_http_status(:ok)

        expect(payload[SMMSService::SMMS_STORE + 0]).to eq(['k4', 'v5'])
        expect(payload[SMMSService::SMMS_STORE + 1]).to eq(['k5', 'v5'])
        expect(payload[SMMSService::SMMS_STORE + 2]).to eq(['k1', 'v1'])
        expect(payload[SMMSService::SMMS_STORE + 3]).to eq(['k2', 'v2'])
        expect(payload[SMMSService::SMMS_STORE + 4]).to eq(['k3', 'v3'])
      end
    end
  end
end
