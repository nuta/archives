require 'rails_helper'

RSpec.describe AppStoresController, type: :controller do
  let(:user) { create(:user) }
  let(:app) { create(:app, user: user) }
  before(:each) { login(user) }

  describe "GET #index" do
    it "returns a success response" do
      create(:app_store, app: app)
      get :index, params: { app_name: app.name }
      expect(response).to be_success
    end
  end


  describe "GET #show" do
    it "returns a success response" do
      app_store = create(:app_store, app: app)
      get :show, params: { app_name: app.name, key: app_store.key }
      expect(response).to be_success
    end
  end

  describe "POST #create" do
    context "with valid params" do
      it "creates a new app_store" do
        expect {
          post :create, params: { app_name: app.name, store: attributes_for(:app_store) }
        }.to change(App, :count).by(1)
      end

      it "renders a JSON response with the new app_store" do
        post :create, params: { app_name: app.name, store: attributes_for(:app_store) }
        expect(response).to have_http_status(:created)
        expect(response.content_type).to eq('application/json')
        expect(response.location).to eq(app_store_url(app_name: app.name, key: AppStore.last.key))
      end
    end

    context "with invalid params" do
      it "renders a JSON response with errors for the new app_store" do
        app_store = attributes_for(:app_store)
        app_store[:data_type] = "invalid"
        post :create, params: { app_name: app.name, store: app_store }
        expect(response).to have_http_status(:unprocessable_entity)
        expect(response.content_type).to eq('application/json')
      end
    end
  end

  describe "PUT #update" do
    context "with valid params" do
      it "updates the requested app_store" do
        app_store = create(:app_store, app: app)
        valid = { comment: "this is comment :D" }
        put :update, params: { app_name: app.name, key: app_store.key, store: valid }
        app_store.reload

        expect(response).to be_success
      end

      it "renders a JSON response with the app_store" do
        app_store = create(:app_store, app: app)

        put :update, params: { app_name: app.name, key: app_store.key, store: attributes_for(:app_store) }
        expect(response).to have_http_status(:ok)
        expect(response.content_type).to eq('application/json')
      end
    end

    context "with invalid params" do
      it "renders a JSON response with errors for the app_store" do
        app_store = create(:app_store, app: app)
        invalid = { data_type: 'good morning' }

        put :update, params: { app_name: app.name, key: app_store.key, store: invalid }
        expect(response).to have_http_status(:unprocessable_entity)
        expect(response.content_type).to eq('application/json')
      end
    end
  end

  describe "DELETE #destroy" do
    it "destroys the requested app_store" do
      app_store = create(:app_store, app: app)
      expect {
        delete :destroy, params: { app_name: app.name, key: app_store.key }
      }.to change(AppStore, :count).by(-1)
    end
  end
end
