require "rails_helper"

RSpec.describe "Authentication", type: :request do
  def invoke()
    headers = {
      'Content-Type' => 'application/json'
    }

    post path, headers: headers, params: params
  end

  describe "sign in" do
    let!(:user) { create(:user) }

    it "returns user data" do
      post  '/api/v1/auth/sign_in', params: { username: user.username, password: user.password }
      expect(response).to have_http_status(:ok)
      expect(JSON.parse(response.body)).to include('data')
      expect(JSON.parse(response.body)['data']).to include('username', 'email')
    end

    it "returns credentials" do
      post  '/api/v1/auth/sign_in', params: { username: user.username, password: user.password }
      expect(response).to have_http_status(:ok)
      expect(response.headers).to include('uid', 'access-token', 'access-token-secret')
    end
  end

  describe "sign up" do
    let!(:new_user) { attributes_for(:user) }
    before do
      stub_request(:post, 'https://www.google.com/recaptcha/api/siteverify')
        .with(body: { remoteip: '127.0.0.1', response: 'good-response', secret: 'good-secret' })
        .to_return(status: 200, body: { success: true }.to_json)

      stub_request(:post, 'https://www.google.com/recaptcha/api/siteverify')
        .with(body: { remoteip: '127.0.0.1', response: 'bad-response', secret: 'good-secret' })
        .to_return(status: 200, body: { success: false }.to_json)
    end

    context "with valid response" do
      it "creates a new user" do
        expect {
          post '/api/v1/auth', params: {
            username: new_user[:username],
            email: new_user[:email],
            password: new_user[:password],
            recaptcha: 'good-response',
            agree_tos: 'yes'
          }

          expect(response).to have_http_status(:ok)
        }.to change(User, :count).by(1)
      end
    end

    context "with invalid reCAPTCHA" do
      it "does not create a new user" do
        expect {
          post  '/api/v1/auth', params: {
            username: new_user['username'],
            email: new_user['email'],
            password: new_user['password'],
            recaptcha: 'bad-response',
            agree_tos: 'yes'
          }

          expect(response).to have_http_status(:unprocessable_entity)
        }.to change(User, :count).by(0)
      end
    end

    context "without tos agreement" do
      it "does not create a new user" do
        expect {
          post  '/api/v1/auth', params: {
            username: new_user['username'],
            email: new_user['email'],
            password: new_user['password'],
            recaptcha: 'bad-response'
          }

          expect(response).to have_http_status(:unprocessable_entity)
        }.to change(User, :count).by(0)
      end
    end
  end


  describe "account deletion" do
    let!(:user) { create(:heavy_user) }

    it "destroys an user" do
      expect {
        delete '/api/v1/auth', headers: user.create_new_auth_token
        expect(response).to have_http_status(:ok)
      }.to change(User, :count).by(-1)
    end
  end
end
