require "rails_helper"

RSpec.describe Apps::DeploymentsController, type: :routing do
  describe "routing" do
    let(:app) { create(:app) }
    let(:deployment) { create(:deployment, app: app) }

    it "routes to #index" do
      expect(:get => "#{api_prefix}/apps/#{app.name}/deployments").to \
        route_to("apps/deployments#index", app_name: app.name)
    end

    it "routes to #show" do
      expect(:get => "#{api_prefix}/apps/#{app.name}/deployments/#{deployment.version}").to \
        route_to("apps/deployments#show", app_name: app.name, version: deployment.version.to_s)
    end

    it "routes to #create" do
      expect(:post => "#{api_prefix}/apps/#{app.name}/deployments").to \
        route_to("apps/deployments#create", app_name: app.name)
    end
  end
end
