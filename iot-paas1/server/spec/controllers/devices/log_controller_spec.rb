require 'rails_helper'

RSpec.describe Devices::LogController, type: :controller do
  let(:user) { create(:user) }
  before { login(user) }

  describe '#index' do
    let!(:device) { create(:device, user: user) }
    let(:lines) { ['hello world!', 'bar', 'baz'] }

    before do
      device.append_log(lines.join("\n"))
    end

    it 'returns log' do
      get :index, params: { device_name: device.name }
      log = JSON.parse(response.body)
      lines.each_with_index do |line, i|
        expect(log['lines'][i]['index']).to eq(i.to_s)
        expect(log['lines'][i]['device']).to eq(device.name)
        expect(log['lines'][i]['body']).to eq(line)
      end
    end
  end
end
