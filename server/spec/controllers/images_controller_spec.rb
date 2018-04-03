require 'rails_helper'

RSpec.describe ImagesController, type: :controller do
  describe '#os_image' do
    let!(:device) { create(:device) }
    let(:os_version) { 'v0.3.0' }
    let(:image_url) { MakeStack.releases[os_version][:assets][device.device_type][:url] }

    it 'returns os image' do
      get 'os_image', params: {
        device_id: device.device_id,
        version: os_version,
        device_type: device.device_type
      }

      expect(response).to have_http_status(:found)
      expect(response).to redirect_to(image_url)
    end

    context 'unknown device' do
      let!(:device2) { build_stubbed(:device) }

      it 'returns :not_found' do
        get 'os_image', params: {
          device_id: device2.device_id,
          version: 'a',
          device_type: device2.device_type
        }

        expect(response).to have_http_status(:forbidden)
      end
    end
  end

  describe '#app_image' do
    let!(:device) { create(:device) }
    let!(:deployments) { create_list(:deployment, 7, app: device.app) }
    let(:latest_deployment) { deployments[-1] }

    context "associated to an app with deployments" do
      it "returns app image" do
        get "app_image", params: {
          device_id: device.device_id,
          version: latest_deployment.version
        }

        expect(response).to have_http_status(:ok)
        expect(response.body).to eq(latest_deployment.image)
      end
    end

    context "associated to an app with no deployments" do
      it "returns :not_found" do
        device = create(:device)

        get "app_image", params: {
          device_id: device.device_id,
          version: '1'
        }

        expect(response).to have_http_status(:not_found)
        expect(response.body).to eq('')
      end
    end

    context "not associated to any apps" do
      it "returns 404" do
        device2 = create(:device, app: nil)
        get "app_image", params: {
          device_id: device2.device_id,
          version: latest_deployment.version
        }

        expect(response).to have_http_status(:not_found)
        expect(response.body).to eq('')
      end
    end
  end
end
