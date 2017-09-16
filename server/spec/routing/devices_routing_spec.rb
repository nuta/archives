require "rails_helper"

RSpec.describe DevicesController, type: :routing do
  describe "routing" do
    let(:device) { create(:device) }

    it "routes to #index" do
      expect(:get => "#{api_prefix}/devices").to route_to("devices#index")
    end

    it "routes to #show" do
      expect(:get => "#{api_prefix}/devices/#{device.name}").to \
        route_to("devices#show", device_name: device.name)
    end

    it "routes to #create" do
      expect(:post => "#{api_prefix}/devices").to route_to("devices#create")
    end

    it "routes to #update via PUT" do
      expect(:put => "#{api_prefix}/devices/#{device.name}").to \
        route_to("devices#update", device_name: device.name)
    end

    it "routes to #update via PATCH" do
      expect(:patch => "#{api_prefix}/devices/#{device.name}").to \
        route_to("devices#update", device_name: device.name)
    end

    it "routes to #destroy" do
      expect(:delete => "#{api_prefix}/devices/#{device.name}").to \
        route_to("devices#destroy", device_name: device.name)
    end
  end
end
