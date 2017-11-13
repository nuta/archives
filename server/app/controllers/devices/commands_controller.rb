class Devices::CommandsController < Devices::ApplicationController
  before_action :set_device

  def create
    @device.send_command!(params[:command], params[:arg])
  end
end
