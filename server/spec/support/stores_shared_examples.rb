require 'rails_helper'

RSpec.shared_examples 'store' do
  let(:model) { described_class.name.demodulize.gsub('sController', '').constantize }
  let(:model_name)    { model.name.underscore.to_sym }
  let(:parent_model) { described_class.name.gsub('StoresController', '').constantize }
  let(:parent_model_name) { parent_model.name.underscore.to_sym }
  let(:parent_model_param)  { (parent_model.name.underscore + '_name').to_sym }

  let(:user) { create(:user) }
  let(:parent) { create(parent_model_name, user: user) }
  before(:each) { login(user) }

  describe "GET #index" do
    it "returns a success response" do
      create(model_name, parent_model_name => parent)
      get :index, params: { parent_model_param => parent.name }
      expect(response).to be_success
    end
  end

  describe "GET #show" do
    it "returns a success response" do
      store = create(model_name, parent_model_name => parent)
      get :show, params: { parent_model_param => parent.name, key: store.key }
      expect(response).to be_success
    end
  end

  describe "POST #create" do
    context "with valid params" do
      it "creates a new store" do
        expect {
          post :create, params: { parent_model_param => parent.name, store: attributes_for(model_name) }
        }.to change(model, :count).by(1)
      end

      it "renders a JSON response with the new store" do
        post :create, params: { parent_model_param => parent.name, store: attributes_for(model_name) }
        expect(response).to have_http_status(:created)
        expect(response.content_type).to eq('application/json')
      end
    end

    context "with invalid params" do
      it "renders a JSON response with errors for the new store" do
        store = attributes_for(model_name)
        store[:data_type] = "invalid"
        post :create, params: { parent_model_param => parent.name, store: store }
        expect(response).to have_http_status(:unprocessable_entity)
        expect(response.content_type).to eq('application/json')
      end
    end
  end

  describe "PUT #update" do
    context "with valid params" do
      it "updates the requested store" do
        store = create(model_name, parent_model_name => parent)
        valid = { comment: "this is comment :D" }
        put :update, params: { parent_model_param => parent.name, key: store.key, store: valid }
        store.reload

        expect(response).to be_success
      end

      it "renders a JSON response with the store" do
        store = create(model_name, parent_model_name => parent)

        put :update, params: { parent_model_param => parent.name, key: store.key, store: attributes_for(model_name) }
        expect(response).to have_http_status(:ok)
        expect(response.content_type).to eq('application/json')
      end
    end

    context "with invalid params" do
      it "renders a JSON response with errors for the store" do
        store = create(model_name, parent_model_name => parent)
        invalid = { data_type: 'good morning' }

        put :update, params: { parent_model_param => parent.name, key: store.key, store: invalid }
        expect(response).to have_http_status(:unprocessable_entity)
        expect(response.content_type).to eq('application/json')
      end
    end
  end

  describe "DELETE #destroy" do
    it "destroys the requested store" do
      store = create(model_name, parent_model_name => parent)
      expect {
        delete :destroy, params: { parent_model_param => parent.name, key: store.key }
      }.to change(model, :count).by(-1)
    end
  end
end
