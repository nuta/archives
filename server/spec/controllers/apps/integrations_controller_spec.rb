require 'rails_helper'

RSpec.describe Apps::IntegrationsController, type: :controller do
  let(:user) { create(:user) }
  let(:app) { create(:app, user: user) }
  before(:each) { login(user) }

  describe "GET #index" do
    it "returns a success response" do
      create(:integration, app: app)
      get :index, params: { app_name: app.name }
      expect(response).to be_successful
    end
  end


  describe "GET #show" do
    it "returns a success response" do
      integration = create(:integration, app: app)
      get :show, params: { app_name: app.name, name: integration.name }
      expect(response).to be_successful
    end
  end

  describe "POST #create" do
    context "with valid params" do
      it "creates a new integration" do
        expect {
          post :create, params: { app_name: app.name, integration: attributes_for(:integration) }
        }.to change(App, :count).by(1)
      end

      it "renders a JSON response with the new integration" do
        post :create, params: { app_name: app.name, integration: attributes_for(:integration) }
        expect(response).to have_http_status(:created)
        expect(response.content_type).to eq('application/json')
        expect(response.location).to eq(app_integration_url(app_name: app.name, name: Integration.last.name))
      end
    end

    context "with invalid params" do
      it "renders a JSON response with errors for the new integration" do
        integration = attributes_for(:integration)
        integration[:config] = "invalid"
        post :create, params: { app_name: app.name, integration: integration }
        expect(response).to have_http_status(:unprocessable_entity)
        expect(response.content_type).to eq('application/json')
      end
    end
  end

  describe "PUT #update" do
    context "with valid params" do
      it "updates the requested integration" do
        integration = create(:integration, app: app)
        valid = { comment: "this is comment :D" }
        put :update, params: { app_name: app.name, name: integration.name, integration: valid }
        integration.reload

        expect(response).to be_successful
      end

      it "renders a JSON response with the integration" do
        integration = create(:integration, app: app)

        put :update, params: { app_name: app.name, name: integration.name, integration: attributes_for(:integration) }
        expect(response).to have_http_status(:ok)
        expect(response.content_type).to eq('application/json')
      end
    end

    context "with invalid params" do
      it "renders a JSON response with errors for the integration" do
        integration = create(:integration, app: app)
        invalid = { config: { service: 'slack', webhook_url: "file://invalid-url" }.to_json }

        put :update, params: { app_name: app.name, name: integration.name, integration: invalid }
        expect(response).to have_http_status(:unprocessable_entity)
        expect(response.content_type).to eq('application/json')
      end
    end
  end

  describe "DELETE #destroy" do
    it "destroys the requested integration" do
      integration = create(:integration, app: app)
      expect {
        delete :destroy, params: { app_name: app.name, name: integration.name }
      }.to change(Integration, :count).by(-1)
    end
  end
end
