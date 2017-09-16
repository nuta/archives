require "rails_helper"

RSpec.describe IntegrationsController, type: :routing do
  describe "routing" do
    let(:app) { create(:app) }
    let(:integration) { create(:integration, app: app) }

    it "routes to #index" do
      expect(:get => "#{api_prefix}/apps/#{app.name}/integrations").to \
        route_to("integrations#index", app_name: app.name)
    end

    it "routes to #show" do
      expect(:get => "#{api_prefix}/apps/#{app.name}/integrations/#{integration.service}").to \
        route_to("integrations#show", app_name: app.name, service: integration.service)
    end

    it "routes to #create" do
      expect(:post => "#{api_prefix}/apps/#{app.name}/integrations").to \
        route_to("integrations#create", app_name: app.name)
    end

    it "routes to #update via PUT" do
      expect(:put => "#{api_prefix}/apps/#{app.name}/integrations/#{integration.service}").to \
        route_to("integrations#update", app_name: app.name, service: integration.service)
    end

    it "routes to #update via PATCH" do
      expect(:patch => "#{api_prefix}/apps/#{app.name}/integrations/#{integration.service}").to \
        route_to("integrations#update", app_name: app.name, service: integration.service)
    end

    it "routes to #destroy" do
      expect(:delete => "#{api_prefix}/apps/#{app.name}/integrations/#{integration.service}").to \
        route_to("integrations#destroy", app_name: app.name, service: integration.service)
    end

  end
end
