require "rails_helper"

RSpec.describe Apps::StoresController, type: :routing do
  describe "routing" do
    let(:app) { create(:app) }
    let(:app_store) { create(:app_store, app: app) }

    it "routes to #index" do
      expect(:get => "#{api_prefix}/apps/#{app.name}/stores").to \
        route_to("apps/stores#index", app_name: app.name)
    end

    it "routes to #show" do
      expect(:get => "#{api_prefix}/apps/#{app.name}/stores/#{app_store.key}").to \
        route_to("apps/stores#show", app_name: app.name, key: app_store.key)
    end

    it "routes to #create" do

      expect(:post => "#{api_prefix}/apps/#{app.name}/stores").to \
        route_to("apps/stores#create", app_name: app.name)
    end

    it "routes to #update via PUT" do
      expect(:put => "#{api_prefix}/apps/#{app.name}/stores/#{app_store.key}").to \
        route_to("apps/stores#update", app_name: app.name, key: app_store.key)
    end

    it "routes to #update via PATCH" do
      expect(:patch => "#{api_prefix}/apps/#{app.name}/stores/#{app_store.key}").to \
        route_to("apps/stores#update", app_name: app.name, key: app_store.key)
    end

    it "routes to #destroy" do
      expect(:delete => "#{api_prefix}/apps/#{app.name}/stores/#{app_store.key}").to \
        route_to("apps/stores#destroy", app_name: app.name, key: app_store.key)
    end
  end
end
