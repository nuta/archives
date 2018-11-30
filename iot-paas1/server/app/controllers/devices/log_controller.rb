class Devices::LogController < Devices::ApplicationController
  before_action :set_device

    def index
      @log = @device.get_log(params[:since])
    end
  end
