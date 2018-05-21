require 'rails_helper'

RSpec.describe SmmsController, type: :controller do
  describe '#http' do
    context 'invalid payload' do
      it 'returns :bad_request' do
        @request.headers['Authorization'] = 'SMMS a b'
        post 'http', body: ''
        expect(response).to have_http_status(:bad_request)
      end
    end
  end
end
