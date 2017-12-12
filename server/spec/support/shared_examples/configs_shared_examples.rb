require 'rails_helper'

RSpec.shared_examples 'config controller' do
  let(:model) { described_class.name.demodulize.gsub('sController', '').constantize }
  let(:model_name)    { model.name.underscore.to_sym }
  let(:owner_model) { described_class.name.gsub('s::ConfigsController', '').constantize }
  let(:owner_name)    { owner_model.name.underscore.to_sym }
  let(:path_name)  { (owner_name.to_s + '_name').to_sym }

  let(:user) { create(:user) }
  let(:owner) { create(owner_name, user: user) }
  before(:each) { login(user) }

  describe "GET #index" do
    it "returns a success response" do
      create(:config, owner: owner)
      get :index, params: { path_name => owner.name }
      expect(response).to be_success
    end
  end
  describe "GET #show" do
    it "returns a success response" do
      config = create(:config, owner: owner)
      get :show, params: { path_name => owner.name, key: config.key }
      expect(response).to be_success
    end
  end

  describe "PUT #update" do
    context "with valid params" do
      it "creates a new config" do
        config = attributes_for(model_name)
        expect {
          put :update, params: { path_name => owner.name, key: config[:key], config: config }
        }.to change(model, :count).by(1)
      end

      it "updates the requested config" do
        config = create(:config, owner: owner)
        valid = { comment: "this is comment :D" }
        put :update, params: { path_name => owner.name, key: config.key, config: valid }
        config.reload

        expect(response).to be_success
      end

      it "renders a JSON response with the config" do
        config = create(:config, owner: owner)

        put :update, params: { path_name => owner.name, key: config.key, config: attributes_for(model_name) }
        expect(response).to have_http_status(:ok)
        expect(response.content_type).to eq('application/json')
      end
    end

    context "with invalid params" do
      it "renders a JSON response with errors for the new config" do
        config = attributes_for(model_name)
        config[:data_type] = "invalid"
        put :update, params: { path_name => owner.name, key: config[:key], config: config }
        expect(response).to have_http_status(:unprocessable_entity)
        expect(response.content_type).to eq('application/json')
      end

      it "renders a JSON response with errors for the config" do
        config = create(:config, owner: owner)
        invalid = { data_type: 'good morning' }

        put :update, params: { path_name => owner.name, key: config.key, config: invalid }
        expect(response).to have_http_status(:unprocessable_entity)
        expect(response.content_type).to eq('application/json')
      end
    end
  end

  describe "DELETE #destroy" do
    it "destroys the requested config" do
      config = create(:config, owner: owner)
      expect {
        delete :destroy, params: { path_name => owner.name, key: config.key }
      }.to change(model, :count).by(-1)
    end
  end
end
