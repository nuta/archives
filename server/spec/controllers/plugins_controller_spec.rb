require 'rails_helper'

RSpec.describe PluginsController, type: :controller do
  let(:user) { create(:user) }
  before(:each) { login(user) }

  describe "GET #download" do
    context "normal" do
      let(:releases)  { { tag_name: 'v0.2.0' } }
      let(:plugin_zip)  { "PKfoo" }
      before do
        stub_request(:get, "https://api.github.com/repos/makestack/homekit-plugin/releases/latest")
        .to_return(status: 200, body: releases.to_json, headers: {})

        stub_request(:get, "https://github.com/makestack/homekit-plugin/releases/download/v0.2.0/homekit-plugin-v0.2.0.plugin.zip")
          .to_return(status: 200, body: plugin_zip, headers: {})
      end

      it 'downloads a pluin zip file from GitHub' do
        get :download, params: {
          org: 'makestack',
           repo: 'homekit-plugin',
            name: 'homekit-plugin'
        }

        expect(response).to have_http_status(:ok)
        expect(response.body).to eq(plugin_zip)
      end
    end

    context "too large zip" do
      let(:releases)  { { tag_name: 'v0.3.0' } }
      before do
        stub_request(:get, "https://api.github.com/repos/makestack/homekit-plugin/releases/latest")
        .to_return(status: 200, body: releases.to_json, headers: {})

        stub_request(:get, "https://github.com/makestack/homekit-plugin/releases/download/v0.3.0/homekit-plugin-v0.3.0.plugin.zip")
          .to_return(status: 200, body: 'a' * 1024 * 1024 * 16)
      end

      it 'downloads a pluin zip file from GitHub' do
        expect {
          get :download, params: {
            org: 'makestack',
             repo: 'homekit-plugin',
              name: 'homekit-plugin'
          }

          expect(response).to have_http_status(:ok)
        }.to raise_exception(RuntimeError)
      end
    end
  end
end
