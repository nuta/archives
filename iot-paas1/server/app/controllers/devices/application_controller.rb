class Devices::ApplicationController < ApplicationController
  def set_device
    @device = @user.devices.find_by_name!(params[:device_name])
  end
end
