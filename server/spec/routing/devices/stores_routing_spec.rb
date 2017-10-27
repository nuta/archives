require "rails_helper"

RSpec.describe Devices::StoresController, type: :routing do
  describe "routing" do
    let(:device) { create(:device) }
    let(:store) { create(:device_store) }

    it "routes to #index" do
      expect(:get => "#{api_prefix}/devices/#{device.name}/stores").to \
        route_to("devices/stores#index", device_name: device.name)
    end

    it "routes to #show" do
      expect(:get => "#{api_prefix}/devices/#{device.name}/stores/#{store.key}").to \
        route_to("devices/stores#show", device_name: device.name, key: store.key)
    end

    it "routes to #create" do
      expect(:post => "#{api_prefix}/devices/#{device.name}/stores").to \
        route_to("devices/stores#create", device_name: device.name)
    end

    it "routes to #update via PUT" do
      expect(:put => "#{api_prefix}/devices/#{device.name}/stores/#{store.key}").to \
        route_to("devices/stores#update", device_name: device.name, key: store.key)
    end

    it "routes to #update via PATCH" do
      expect(:patch => "#{api_prefix}/devices/#{device.name}/stores/#{store.key}").to \
        route_to("devices/stores#update", device_name: device.name, key: store.key)
    end

    it "routes to #destroy" do
      expect(:delete => "#{api_prefix}/devices/#{device.name}/stores/#{store.key}").to \
        route_to("devices/stores#destroy", device_name: device.name, key: store.key)
    end

  end
end
