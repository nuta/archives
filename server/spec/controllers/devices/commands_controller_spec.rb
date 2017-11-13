require 'rails_helper'

RSpec.describe Devices::CommandsController, type: :controller do
  let(:user) { create(:user) }
  before(:each) { login(user) }

  describe "POST #create" do
    context "with valid params" do
      let!(:device) { create(:device, user: user) }
      let(:command) { 'abc' }
      let(:arg) { ['const x = new I2C({ address: 0x40 })', 'x.write([0x00])'].to_json }

      it "creates a new store" do
        expect {
          post :create, params: { device_name: device.name, command: command, arg: arg }
        }.to change(Store, :count).by(1)

        command_store = Store.where(owner_type: 'Device', owner_id: device.id).last
        expect(command_store.key).to start_with('>')
        expect(command_store.value).to start_with(arg)
      end
    end
  end
end
