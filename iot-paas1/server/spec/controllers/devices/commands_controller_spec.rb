require 'rails_helper'

RSpec.describe Devices::CommandsController, type: :controller do
  let(:user) { create(:user) }
  before(:each) { login(user) }

  describe "POST #create" do
    context "with valid params" do
      let!(:device) { create(:device, user: user) }
      let(:command) { 'abc' }
      let(:arg) { ['New Display Message'].to_json }

      it "creates a new store" do
        expect {
          post :create, params: { device_name: device.name, command: command, arg: arg }
        }.to change(device.pending_commands, :count).by(1)

        command_store = Config.where(owner_type: 'Device', owner_id: device.id).last
        expect(device.pending_commands[0][:id]).to be_a(Numeric)
        expect(device.pending_commands[0][:key]).to eq(command)
        expect(device.pending_commands[0][:arg]).to include(arg)
      end
    end
  end
end
