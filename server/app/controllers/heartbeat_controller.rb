class HeartbeatController < ApplicationController
  skip_before_action :authenticate

  def heartbeat
    device = SMMSService.receive(request.body.read)
    render body: SMMSService.payload_for(device)
  end

  def os_image
    head :not_implemented
  end
  
  def app_image
    @device = Device.authenticate(params[:device_id])
    unless @device
      head :forbidden
      return
    end

    render type: 'application/zip', body: @device.app_image(params[:version])
  end
 end
