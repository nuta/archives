require 'rails_helper'

RSpec.describe ImagesController, type: :controller do
  describe '#os_image' do
    let!(:device) { create(:device) }
    let(:mock_osimage_a) { 'mock image A' }
    let(:mock_osimage_c) { 'mock image C' }

    before do
      # create mock image cache
      cache_dir = "#{Rails.root}/tmp/cache/downloads"
      FileUtils.mkdir_p(cache_dir)

      image_url = 'http://localhost:8100/repos/os/mock.img'
      path = File.join(cache_dir, OpenSSL::Digest::SHA1.hexdigest(image_url))
      File.open(path, 'w') do |f|
        f.write(mock_osimage_a)
      end
    end

    before do
      stub_request(:get, 'http://localhost:8100/repos/os/mock2.img')
        .to_return(status: 200, body: mock_osimage_c)
    end

    after(:each) do
      FileUtils.rm_r("#{Rails.root}/tmp/cache/downloads")
    end

    context 'hit cache' do
      it 'returns os image' do
        get 'os_image', params: {
          device_id: device.device_id,
          version: 'a',
          os: 'linux',
          device_type: device.device_type
        }

        expect(response).to have_http_status(:ok)
        expect(response.body).to eq(mock_osimage_a)
      end
    end

    context 'miss cache' do
      it 'downloads and returns os image' do
        get 'os_image', params: {
          device_id: device.device_id,
          version: 'c',
          os: 'linux',
          device_type: device.device_type
        }

        expect(response).to have_http_status(:ok)
        expect(response.body).to eq(mock_osimage_c)
      end
    end

    context 'not found' do
      it 'returns :not_found' do
        get 'os_image', params: {
          device_id: device.device_id,
          version: 'xyz',
          os: 'linux',
          device_type: device.device_type
        }

        expect(response).to have_http_status(:not_found)
      end
    end

    context 'unknown device' do
      let!(:device2) { build_stubbed(:device) }

      it 'returns :not_found' do
        get 'os_image', params: {
          device_id: device2.device_id,
          version: 'a',
          os: 'linux',
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
