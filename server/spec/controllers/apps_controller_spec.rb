require 'rails_helper'

RSpec.describe AppsController, type: :controller do
  let(:user) { create(:user) }
  before(:each) { login(user) }

  describe "GET #index" do
    it "returns a success response" do
      create(:app, user: user)
      get :index
      expect(response).to be_success
    end
  end


  describe "GET #show" do
    it "returns a success response" do
      app = create(:app, user: user)
      get :show, params: {name: app.name}
      expect(response).to be_success
    end
  end

  describe "POST #create" do
    context "with valid params" do
      it "creates a new app" do
        expect {
          post :create, params: {app: attributes_for(:app)}
        }.to change(App, :count).by(1)
      end

      it "renders a JSON response with the new app" do
        post :create, params: {app: attributes_for(:app)}
        expect(response).to have_http_status(:created)
        expect(response.content_type).to eq('application/json')
        expect(response.location).to eq(app_url(App.last))
      end
    end

    context "with invalid params" do
      it "renders a JSON response with errors for the new app" do
        app = attributes_for(:app)
        app[:api] = "unsupported-one"

        post :create, params: {app: app}
        expect(response).to have_http_status(:unprocessable_entity)
        expect(response.content_type).to eq('application/json')
      end
    end
  end

  describe "PUT #update" do
    context "with valid params" do
      it "updates the requested app" do
        app = create(:app, user: user)
        valid = { api: "nodejs" }
        put :update, params: {name: app.name, app: valid}
        app.reload

        expect(response).to be_success
      end

      it "renders a JSON response with the app" do
        app = create(:app, user: user)

        put :update, params: {name: app.name, app: attributes_for(:app)}
        expect(response).to have_http_status(:ok)
        expect(response.content_type).to eq('application/json')
      end
    end

    context "with invalid params" do
      it "renders a JSON response with errors for the app" do
        app = create(:app, user: user)
        invalid = { api: "unsupported-one" }

        put :update, params: {name: app.name, app: invalid}
        expect(response).to have_http_status(:unprocessable_entity)
        expect(response.content_type).to eq('application/json')
      end
    end
  end

  describe "DELETE #destroy" do
    it "destroys the requested app" do
      app = create(:app, user: user)
      expect {
        delete :destroy, params: {name: app.name}
      }.to change(App, :count).by(-1)
    end
  end
end
