require "rails_helper"

RSpec.describe "SMMS over HTTP", type: :request do
  def build_payload(messages)
    payload = ''
    messages.each do |type, data|
      lenbuf = SMMSService.send(:generate_variable_length, data)
      payload += [type].pack('C') + lenbuf + data
    end

    total_length = SMMSService.send(:generate_variable_length, payload)
    header = [SMMSService::SMMS_VERSION << 4].pack('C') + total_length
    header + payload
  end

  def send_msg(device, device_info:, app_version: 0, os_version: 'a', log: '')
    payload = build_payload({
      SMMSService::SMMS_DEVICE_INFO_MSG => [device_info].pack('C'),
      SMMSService::SMMS_OS_VERSION_MSG => os_version,
      SMMSService::SMMS_APP_VERSION_MSG => app_version.to_s,
      SMMSService::SMMS_DEVICE_ID_MSG => device.device_id,
      SMMSService::SMMS_LOG_MSG => log
    })

    timestamp, hmac = SMMSService::sign(device, payload)

    headers = {
      'Authorization' => "SMMS #{timestamp} #{hmac}",
      'Content-Type' => 'application/octet-stream'
    }

    post '/api/v1/smms', headers: headers, params: payload

    begin
      SMMSService.send(:parse_payload, response.body)
    rescue => e
      raise "something went wrong with parse_payload: #{e}"
    end
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

        expect(payload[:app_version]).to eq(deployments[-1].version.to_s)
      end
    end

    context "invalid authetication header" do
      it "returns :unauthorized" do
        device = create(:device, app: nil)
        post '/api/v1/smms', headers: { authetication: 'SMMS 0001' }
        expect(response).to have_http_status(:unauthorized)
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
    let(:image_url) { MakeStack.os_releases[os_version][:linux][:assets][device.device_type][:url] }

    context "image exists" do
      it "returns os image" do
        get "/api/v1/images/os/#{device.device_id}/#{os_version}/linux/#{device.device_type}"
        expect(response).to redirect_to(image_url)
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
        stores = create_list(:device_store, 5, owner: device)

        payload = send_msg(device, device_info: 1) # booting
        expect(response).to have_http_status(:ok)

        stores.sort_by(&:key).each_with_index do |store, i|
          expect(payload[:stores][i]).to eq({ key: store.key, value: store.value })
        end
      end
    end

    context "some app stores are set" do
      it "returns stores" do
        device = create(:device, app: create(:app))
        stores = create_list(:app_store, 5, owner: device.app)

        payload = send_msg(device, device_info: 1) # booting
        expect(response).to have_http_status(:ok)

        stores.sort_by(&:key).each_with_index do |store, i|
          expect(payload[:stores][i]).to eq({ key: store.key, value: store.value })
        end
      end
    end

    context "some device and app stores are set" do
      it "returns stores" do
        device = create(:device, app: create(:app))
        create(:device_store, owner: device,     key: 'k2', value: 'v2')
        create(:device_store, owner: device,     key: 'k1', value: 'v1')
        create(:device_store, owner: device,     key: 'k3', value: 'v3')
        create(:app_store,    owner: device.app, key: 'k4', value: 'v5')
        create(:app_store,    owner: device.app, key: 'k5', value: 'v5')
        create(:app_store,    owner: device.app, key: 'k1', value: 'v6')

        create(:device_store, owner: create(:device),  key: 'k0', value: 'v3')
        create(:app_store,    owner: create(:app), key: 'abc', value: 'v6')
        create(:device_store, owner: create(:device, user: device.user),  key: 'k0', value: 'v3')
        create(:app_store,    owner: create(:app, user: device.user), key: 'abc', value: 'v6')

        payload = send_msg(device, device_info: 1) # booting
        expect(response).to have_http_status(:ok)

        expect(payload[:stores][0]).to eq({ key: 'k1', value: 'v1' })
        expect(payload[:stores][1]).to eq({ key: 'k2', value: 'v2' })
        expect(payload[:stores][2]).to eq({ key: 'k3', value: 'v3' })
        expect(payload[:stores][3]).to eq({ key: 'k4', value: 'v5' })
        expect(payload[:stores][4]).to eq({ key: 'k5', value: 'v5' })
        expect(payload[:stores][5]).to eq(nil)
      end
    end
  end
end
