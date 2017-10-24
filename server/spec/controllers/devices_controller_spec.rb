require 'rails_helper'

RSpec.describe DevicesController, type: :controller do
  let(:user) { create(:user) }
  before(:each) { login(user) }

  describe "GET #index" do
    it "returns a success response" do
      create(:device, user: user)
      get :index
      expect(response).to be_success
    end
  end

  describe "GET #show" do
    it "returns a success response" do
      device = create(:device, user: user)
      get :show, params: {name: device.name}
      expect(response).to be_success
    end
  end

  describe "POST #create" do
    context "with valid params" do
      it "creates a new device" do
        expect {
          post :create, params: {device: attributes_for(:device)}
        }.to change(Device, :count).by(1)
      end

      it "renders a JSON response with the new device" do
        post :create, params: {device: attributes_for(:device)}
        expect(response).to have_http_status(:created)
        expect(response.content_type).to eq('application/json')
        expect(response.location).to eq(device_url(Device.last))
      end
    end

    context "with invalid params" do
      it "renders a JSON response with errors for the new device" do
        device = attributes_for(:device)
        device[:device_type] = "unsupported-one"

        post :create, params: {device: device}
        expect(response).to have_http_status(:unprocessable_entity)
        expect(response.content_type).to eq('application/json')
      end
    end
  end

  describe "PUT #update" do
    context "with valid params" do
      it "updates the requested device" do
        device = create(:device, user: user)
        valid = { api: "makestack" }
        put :update, params: {name: device.name, device: valid}
        device.reload

        expect(response).to be_success
      end

      it "renders a JSON response with the device" do
        device = create(:device, user: user)

        put :update, params: {name: device.name, device: attributes_for(:device)}
        expect(response).to have_http_status(:ok)
        expect(response.content_type).to eq('application/json')
      end
    end

    context "with invalid params" do
      it "renders a JSON response with errors for the device" do
        device = create(:device, user: user)
        invalid = { device_type: "unsupported-one" }

        put :update, params: {name: device.name, device: invalid}
        expect(response).to have_http_status(:unprocessable_entity)
        expect(response.content_type).to eq('application/json')
      end
    end
  end

  describe "DELETE #destroy" do
    it "destroys the requested device" do
      device = create(:device, user: user)
      expect {
        delete :destroy, params: {name: device.name}
      }.to change(Device, :count).by(-1)
    end
  end
end
