require 'rails_helper'

RSpec.shared_examples 'store controller' do
  let(:model) { described_class.name.demodulize.gsub('sController', '').constantize }
  let(:model_name)    { model.name.underscore.to_sym }
  let(:owner_model) { described_class.name.gsub('s::StoresController', '').constantize }
  let(:owner_name)    { owner_model.name.underscore.to_sym }
  let(:path_name)  { (owner_name.to_s + '_name').to_sym }

  let(:user) { create(:user) }
  let(:owner) { create(owner_name, user: user) }
  before(:each) { login(user) }

  describe "GET #index" do
    it "returns a success response" do
      create(:store, owner: owner)
      get :index, params: { path_name => owner.name }
      expect(response).to be_success
    end
  end
  describe "GET #show" do
    it "returns a success response" do
      store = create(:store, owner: owner)
      get :show, params: { path_name => owner.name, key: store.key }
      expect(response).to be_success
    end
  end

  describe "POST #create" do
    context "with valid params" do
      it "creates a new store" do
        expect {
          post :create, params: { path_name => owner.name, store: attributes_for(model_name) }
        }.to change(model, :count).by(1)
      end

      it "renders a JSON response with the new store" do
        post :create, params: { path_name => owner.name, store: attributes_for(model_name) }
        expect(response).to have_http_status(:created)
        expect(response.content_type).to eq('application/json')
      end
    end

    context "with invalid params" do
      it "renders a JSON response with errors for the new store" do
        store = attributes_for(model_name)
        store[:data_type] = "invalid"
        post :create, params: { path_name => owner.name, store: store }
        expect(response).to have_http_status(:unprocessable_entity)
        expect(response.content_type).to eq('application/json')
      end
    end
  end

  describe "PUT #update" do
    context "with valid params" do
      it "updates the requested store" do
        store = create(:store, owner: owner)
        valid = { comment: "this is comment :D" }
        put :update, params: { path_name => owner.name, key: store.key, store: valid }
        store.reload

        expect(response).to be_success
      end

      it "renders a JSON response with the store" do
        store = create(:store, owner: owner)

        put :update, params: { path_name => owner.name, key: store.key, store: attributes_for(model_name) }
        expect(response).to have_http_status(:ok)
        expect(response.content_type).to eq('application/json')
      end
    end

    context "with invalid params" do
      it "renders a JSON response with errors for the store" do
        store = create(:store, owner: owner)
        invalid = { data_type: 'good morning' }

        put :update, params: { path_name => owner.name, key: store.key, store: invalid }
        expect(response).to have_http_status(:unprocessable_entity)
        expect(response.content_type).to eq('application/json')
      end
    end
  end

  describe "DELETE #destroy" do
    it "destroys the requested store" do
      store = create(:store, owner: owner)
      expect {
        delete :destroy, params: { path_name => owner.name, key: store.key }
      }.to change(model, :count).by(-1)
    end
  end
end
