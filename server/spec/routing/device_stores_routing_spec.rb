require "rails_helper"

RSpec.describe DeviceStoresController, type: :routing do
  describe "routing" do
    let(:device) { create(:device) }
    let(:device_store) { create(:device_store, device: device) }

    it "routes to #index" do
      expect(:get => "#{api_prefix}/devices/#{device.name}/stores").to \
        route_to("device_stores#index", device_name: device.name)
    end

    it "routes to #show" do
      expect(:get => "#{api_prefix}/devices/#{device.name}/stores/#{device_store.key}").to \
        route_to("device_stores#show", device_name: device.name, key: device_store.key)
    end

    it "routes to #create" do
      expect(:post => "#{api_prefix}/devices/#{device.name}/stores").to \
        route_to("device_stores#create", device_name: device.name)
    end

    it "routes to #update via PUT" do
      expect(:put => "#{api_prefix}/devices/#{device.name}/stores/#{device_store.key}").to \
        route_to("device_stores#update", device_name: device.name, key: device_store.key)
    end

    it "routes to #update via PATCH" do
      expect(:patch => "#{api_prefix}/devices/#{device.name}/stores/#{device_store.key}").to \
        route_to("device_stores#update", device_name: device.name, key: device_store.key)
    end

    it "routes to #destroy" do
      expect(:delete => "#{api_prefix}/devices/#{device.name}/stores/#{device_store.key}").to \
        route_to("device_stores#destroy", device_name: device.name, key: device_store.key)
    end

  end
end
