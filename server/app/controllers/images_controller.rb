class ImagesController < ApplicationController
  skip_before_action :authenticate
  before_action :authenticate_device

  def os_image
    image_url = os_image_url(params[:version], params[:os], params[:device_type])
    unless image_url
      head :not_found
      return
    end

    redirect_to image_url
  end

  def app_image
    payload = @device.app_image(params[:version])
    unless payload
      head :not_found
      return
    end

    response.headers["Content-Length"] = payload.length
    render content_type: 'application/zip', body: payload
  end

  private

  def authenticate_device
    @device = Device.authenticate(params[:device_id])
    unless @device
      head :forbidden
      return
    end
  end

  def os_image_url(version, os, deviceTypes)
    MakeStack.releases.dig(version, os, :assets, deviceTypes, :url)
  end
 end
