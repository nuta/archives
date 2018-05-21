require "rails_helper"

RSpec.describe Devices::ConfigsController, type: :routing do
  describe "routing" do
    let(:device) { create(:device) }
    let(:config) { create(:device_config) }

    it "routes to #index" do
      expect(:get => "#{api_prefix}/devices/#{device.name}/configs").to \
        route_to("devices/configs#index", device_name: device.name)
    end

    it "routes to #show" do
      expect(:get => "#{api_prefix}/devices/#{device.name}/configs/#{config.key}").to \
        route_to("devices/configs#show", device_name: device.name, key: config.key)
    end

    it "routes to #update via PUT" do
      expect(:put => "#{api_prefix}/devices/#{device.name}/configs/#{config.key}").to \
        route_to("devices/configs#update", device_name: device.name, key: config.key)
    end

    it "routes to #update via PATCH" do
      expect(:patch => "#{api_prefix}/devices/#{device.name}/configs/#{config.key}").to \
        route_to("devices/configs#update", device_name: device.name, key: config.key)
    end

    it "routes to #destroy" do
      expect(:delete => "#{api_prefix}/devices/#{device.name}/configs/#{config.key}").to \
        route_to("devices/configs#destroy", device_name: device.name, key: config.key)
    end

  end
end
