require 'rails_helper'

RSpec.describe Apps::DeploymentsController, type: :controller do
  let(:user) { create(:user) }
  let(:app) { create(:app, user: user) }
  before(:each) { login(user) }

  describe "GET #index" do
    it "returns a success response" do
      create(:deployment, app: app)
      get :index, params: { app_name: app.name }
      expect(response).to be_success
    end
  end


  describe "GET #show" do
    it "returns a success response" do
      deployment = create(:deployment, app: app)
      get :show, params: { app_name: app.name, version: deployment.version }
      expect(response).to be_success
    end
  end

  describe "POST #create" do
    context "with valid params" do
      it "creates a new deployment" do
        expect {
          post :create, params: { app_name: app.name, deployment: attributes_for(:deployment) }
        }.to change(App, :count).by(1)
      end

      it "renders a JSON response with the new deployment" do
        post :create, params: { app_name: app.name, deployment: attributes_for(:deployment) }
        expect(response).to have_http_status(:created)
        expect(response.content_type).to eq('application/json')
        expect(response.location).to eq(app_deployment_url(app_name: app.name, version: Deployment.last.version))
      end
    end

    context "with invalid params" do
      it "renders a JSON response with errors for the new deployment" do
        deployment = attributes_for(:deployment)
        deployment[:image] = "invalid"
        post :create, params: { app_name: app.name, deployment: deployment }
        expect(response).to have_http_status(:unprocessable_entity)
        expect(response.content_type).to eq('application/json')
      end
    end
  end
end
