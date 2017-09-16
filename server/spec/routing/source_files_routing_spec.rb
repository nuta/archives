require "rails_helper"

RSpec.describe SourceFilesController, type: :routing do
  describe "routing" do
    let(:app) { create(:app) }
    let(:source_file) { create(:source_file, app: app) }

    it "routes to #index" do
      expect(:get => "#{api_prefix}/apps/#{app.name}/files").to \
        route_to("source_files#index", app_name: app.name)
    end

    it "routes to #show" do
      expect(:get => "#{api_prefix}/apps/#{app.name}/files/#{source_file.path}").to \
        route_to("source_files#show", app_name: app.name, path: source_file.path)
    end

    it "routes to #update via PUT" do
      expect(:put => "#{api_prefix}/apps/#{app.name}/files/#{source_file.path}").to \
        route_to("source_files#update", app_name: app.name, path: source_file.path)
    end

    it "routes to #destroy" do
      expect(:delete => "#{api_prefix}/apps/#{app.name}/files/#{source_file.path}").to \
        route_to("source_files#destroy", app_name: app.name, path: source_file.path)
    end
  end
end
