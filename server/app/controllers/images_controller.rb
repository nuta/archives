class ImagesController < ApplicationController
  skip_before_action :authenticate

  def os_image
    image_url = look_for_os_image_url(params[:version], params[:os], params[:device_type])
    unless image_url
      head :not_found
      return
    end

    # TODO: sign_and_set_authorization_header
    redirect_to image_url
  end

  def app_image
    @device = Device.authenticate(params[:device_id])
    unless @device
      head :forbidden
      return
    end

    payload = @device.app_image(params[:version])
    sign_and_set_authorization_header(@device, payload)
    render type: 'application/zip', body: payload
  end

  private

  def look_for_os_image_url(version, os, deviceTypes)
    MakeStack.settings.dig(:os_releases, version, os, deviceTypes, :url)
  end
 end
