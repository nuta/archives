require "rails_helper"

RSpec.describe AppsController, type: :routing do
  describe "routing" do
    let(:app) { create(:app) }

    it "routes to #index" do
      expect(:get => "#{api_prefix}/apps").to route_to("apps#index")
    end

    it "routes to #show" do
      expect(:get => "#{api_prefix}/apps/#{app.name}").to \
        route_to("apps#show", app_name: app.name)
    end

    it "routes to #create" do
      expect(:post => "#{api_prefix}/apps").to route_to("apps#create")
    end

    it "routes to #update via PUT" do
      expect(:put => "#{api_prefix}/apps/#{app.name}").to \
        route_to("apps#update", app_name: app.name)
    end

    it "routes to #update via PATCH" do
      expect(:patch => "#{api_prefix}/apps/#{app.name}").to \
        route_to("apps#update", app_name: app.name)
    end

    it "routes to #destroy" do
      expect(:delete => "#{api_prefix}/apps/#{app.name}").to \
        route_to("apps#destroy", app_name: app.name)
    end
  end
end
