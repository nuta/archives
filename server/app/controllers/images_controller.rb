class ImagesController < ApplicationController
  skip_before_action :authenticate
  before_action :authenticate_device

  def os_image
    image_url = look_for_os_image_url(params[:version], params[:os], params[:device_type])
    unless image_url
      head :not_found
      return
    end

    image_filepath = download_image_url(image_url)

    begin
      sign_and_set_authorization_header(@device, open(image_filepath, 'rb'))
      send_file image_filepath, content_type: 'application/x-os-image'
    rescue Errno::ENOENT
      # The downloaded file could be accidentally removed before send_file
      # by cache cleaning job (not implemented yet, by the way) or something.
      head :processing
    end
  end

  def app_image
    payload = @device.app_image(params[:version])
    unless payload
      head :not_found
      return
    end

    sign_and_set_authorization_header(@device, payload)
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

  def download_image_url(image_url)
    basename = OpenSSL::Digest::SHA1.hexdigest(image_url)
    downloads_dir = "#{Rails.root}/tmp/cache/downloads"
    filepath = File.join(downloads_dir, basename)

    if File.exist?(filepath)
      return filepath
    end

    FileUtils.mkdir_p(downloads_dir)

    tmp_filepath =filepath + SecureRandom.hex(8) + '.downloading'
    download_file(image_url, tmp_filepath)

    # We assume that File.rename() is atomic.
    File.rename(tmp_filepath, filepath)

    filepath
  end

  def download_file(url, filepath)
    File.open(filepath, 'w') do |f|
      block = proc {|resp| resp.read_body {|chunk| f.write(chunk) }}
      RestClient::Request.new(method: :get, url: url, block_response: block).execute
    end
  end

  def look_for_os_image_url(version, os, deviceTypes)
    MakeStack.os_releases.dig(version, os, :assets, deviceTypes, :url)
  end
 end
