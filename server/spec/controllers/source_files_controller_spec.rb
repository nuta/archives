require 'rails_helper'

RSpec.describe SourceFilesController, type: :controller do
  let(:user) { create(:user) }
  let(:app) { create(:app, user: user) }
  before(:each) { login(user) }

  describe "GET #index" do
    it "returns a success response" do
      create(:source_file, app: app)
      get :index, params: { app_name: app.name }
      expect(response).to be_success
    end
  end


  describe "GET #show" do
    it "returns a success response" do
      source_file= create(:source_file, app: app)
      get :show, params: { app_name: app.name, path: source_file.path }
      expect(response).to be_success
    end
  end

  describe "PUT #update" do
    context "with valid params" do
      it "creates a new source_file" do
        source_file = attributes_for(:source_file)
        expect {
          post :update, params: { app_name: app.name, path: source_file[:path], source_file: source_file }
        }.to change(App, :count).by(1)
      end

      it "updates the requested source_file" do
        source_file = create(:source_file, app: app)
        valid = { comment: "this is comment :D" }
        put :update, params: { app_name: app.name, path: source_file.path, source_file: valid }
        source_file.reload

        expect(response).to be_success
      end

      it "renders a JSON response with the source_file" do
        source_file= create(:source_file, app: app)

        put :update, params: { app_name: app.name, path: source_file.path, source_file: attributes_for(:source_file) }
        expect(response).to have_http_status(:ok)
        expect(response.content_type).to eq('application/json')
      end
    end

    context "with invalid params" do
      it "renders a JSON response with errors for the source_file" do
        invalid_path = '////'
        source_file = attributes_for(:source_file, path: invalid_path, app: app)

        put :update, params: { app_name: app.name, path: invalid_path, source_file: source_file }
        expect(response).to have_http_status(:unprocessable_entity)
        expect(response.content_type).to eq('application/json')
      end
    end
  end

  describe "DELETE #destroy" do
    it "destroys the requested source_file" do
      source_file= create(:source_file, app: app)
      expect {
        delete :destroy, params: { app_name: app.name, path: source_file.path }
      }.to change(SourceFile, :count).by(-1)
    end
  end
end
