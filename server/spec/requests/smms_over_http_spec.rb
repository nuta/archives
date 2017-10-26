require "rails_helper"

RSpec.describe "SMMS over HTTP", type: :request do
  def send_msg(device, version: 1, device_info:, app_version: 0, os_version: 'a', log: '')
    payload = {
      SMMSService::SMMS_VERSION => version,
      SMMSService::SMMS_DEVICE_INFO => device_info,
      SMMSService::SMMS_OS_VERSION => os_version,
      SMMSService::SMMS_APP_VERSION => app_version,
      SMMSService::SMMS_DEVICE_ID => device.device_id,
      SMMSService::SMMS_LOG => log
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

  describe "app image endpoint" do
    let(:device) { create(:device) }
    let!(:deployments) { create_list(:deployment, 7, app: device.app) }
    let(:latest_deployment) { deployments[-1] }

    context "associated to an app with deployments" do
      it "returns app image" do
        get "/api/v1/images/app/#{device.device_id}/#{latest_deployment.version}"
        expect(response).to have_http_status(:ok)
        expect(response.body).to eq(latest_deployment.image)
      end
    end

    context "associated to an app with no deployments" do
      it "returns :not_found" do
        device = create(:device)
        get "/api/v1/images/app/#{device.device_id}/1"
        expect(response).to have_http_status(:not_found)
        expect(response.body).to eq('')
      end
    end

    context "not associated to any apps" do
      it "returns 404" do
        device2 = create(:device, app: nil)
        get "/api/v1/images/app/#{device2.device_id}/1"
        expect(response).to have_http_status(:not_found)
        expect(response.body).to eq('')
      end
    end
  end

  describe "logging" do
    let(:device) { create(:device) }
    let!(:deployments) { create_list(:deployment, 7, app: device.app) }
    let(:latest_deployment) { deployments[-1] }


    context "associated to an app" do
      it "stores log" do
        device = create(:device)
        log = "This is first line!\nThe next line!\n"

        payload = send_msg(device, device_info: 1, log: log) # booting
        expect(response).to have_http_status(:ok)
        expect(device.get_log.map{|l| l[:body]}.join("\n")).to include(log.strip)
        expect(device.app.get_log.map{|l| l[:body]}.join("\n")).to include(log.strip)
      end
    end

    context "not associated to any apps" do
      it "returns 404" do
        device2 = create(:device, app: nil)
        get "/api/v1/images/app/#{device2.device_id}/1"
        expect(response).to have_http_status(:not_found)
        expect(response.body).to eq('')
      end
    end
  end

  describe "os image endpoint" do
    let(:device) { create(:device) }
    let!(:deployments) { create_list(:deployment, 7, app: device.app) }
    let(:latest_deployment) { deployments[-1] }
    let(:os_version) { 'a' }
    let(:mock_osimage_content) { 'mock image :D' }

    before do
      # create mock image cache
      cache_dir = "#{Rails.root}/tmp/cache/downloads"
      FileUtils.mkdir_p(cache_dir)

      image_url = 'http://localhost:8100/repos/os/mock.img'
      path = File.join(cache_dir, OpenSSL::Digest::SHA1.hexdigest(image_url))
      File.open(path, 'w') do |f|
        f.write(mock_osimage_content)
      end
    end

    context "associated to an app with deployments" do
      it "returns app image" do
        get "/api/v1/images/os/#{device.device_id}/#{os_version}/linux/#{device.device_type}"
        expect(response).to have_http_status(:ok)
        expect(response.body).to eq(mock_osimage_content)
      end
    end

    context "image not found" do
      it "returns 404" do
        get "/api/v1/images/os/#{device.device_id}/not-found/linux/raspberrypi3"
        expect(response).to have_http_status(:not_found)
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
