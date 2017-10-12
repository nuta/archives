class HeartbeatController < ApplicationController
  skip_before_action :authenticate

  def heartbeat
    device = handle_smms_payload(request.body.read)
    render body: generate_smms_payload_for(device)
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

    render body: @device.app_image(params[:version])
  end
 end
