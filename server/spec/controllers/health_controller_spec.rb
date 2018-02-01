require 'rails_helper'

RSpec.describe HealthController, type: :controller do
  describe "#health" do
    it "returns 200" do
      get :health
      expect(response).to have_http_status(:ok)
    end
  end
end
