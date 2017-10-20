class HeartbeatController < ApplicationController
  skip_before_action :authenticate

  def heartbeat
    device = SMMSService.receive(request.body.read)
    render body: SMMSService.payload_for(device)
  end

  def os_image
    image_url = look_for_os_image_url(params[:os], params[:device_type], params[:version])
    unless image_url
      head :not_found
      return
    end

    redirect_to image_url
  end

  def app_image
    @device = Device.authenticate(params[:device_id])
    unless @device
      head :forbidden
      return
    end

    render type: 'application/zip', body: @device.app_image(params[:version])
  end

  private

  def look_for_os_image_url(os, deviceTypes, version)
    MakeStack.settings.dig(:os_releases, version, os, deviceTypes, :url)
  end
 end
