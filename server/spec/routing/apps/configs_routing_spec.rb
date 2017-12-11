require "rails_helper"

RSpec.describe Apps::ConfigsController, type: :routing do
  describe "routing" do
    let(:app) { create(:app) }
    let(:config) { create(:app_config) }

    it "routes to #index" do
      expect(:get => "#{api_prefix}/apps/#{app.name}/configs").to \
        route_to("apps/configs#index", app_name: app.name)
    end

    it "routes to #show" do
      expect(:get => "#{api_prefix}/apps/#{app.name}/configs/#{config.key}").to \
        route_to("apps/configs#show", app_name: app.name, key: config.key)
    end

    it "routes to #create" do

      expect(:post => "#{api_prefix}/apps/#{app.name}/configs").to \
        route_to("apps/configs#create", app_name: app.name)
    end

    it "routes to #update via PUT" do
      expect(:put => "#{api_prefix}/apps/#{app.name}/configs/#{config.key}").to \
        route_to("apps/configs#update", app_name: app.name, key: config.key)
    end

    it "routes to #update via PATCH" do
      expect(:patch => "#{api_prefix}/apps/#{app.name}/configs/#{config.key}").to \
        route_to("apps/configs#update", app_name: app.name, key: config.key)
    end

    it "routes to #destroy" do
      expect(:delete => "#{api_prefix}/apps/#{app.name}/configs/#{config.key}").to \
        route_to("apps/configs#destroy", app_name: app.name, key: config.key)
    end
  end
end
