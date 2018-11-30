require 'rails_helper'

RSpec.describe Apps::LogController, type: :controller do
  let(:user) { create(:user) }
  before { login(user) }

  describe '#index' do
    let!(:app) { create(:app, user: user) }
    let!(:device1) { create(:device, user: user, app: app) }
    let!(:device2) { create(:device, user: user, app: app) }
    let(:lines1) { ['hello world!', 'bar', 'baz'] }
    let(:lines2) { ['guten tag!', 'hoge', 'fuga'] }

    before do
      device1.append_log(lines1.join("\n"))
      device2.append_log(lines2.join("\n"))
    end

    it 'returns log' do
      get :index, params: { app_name: app.name }
      log = JSON.parse(response.body)

      lines1.each_with_index do |line, i|
        expect(log['lines'][i]['index']).to eq(i.to_s)
        expect(log['lines'][i]['device']).to eq(device1.name)
        expect(log['lines'][i]['body']).to eq(line)
      end

      lines2.each_with_index do |line, i|
        expect(log['lines'][3 + i]['index']).to eq(i.to_s)
        expect(log['lines'][3 + i]['device']).to eq(device2.name)
        expect(log['lines'][3 + i]['body']).to eq(line)
      end
    end
  end
end
