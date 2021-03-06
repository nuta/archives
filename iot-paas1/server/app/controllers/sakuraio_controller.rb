class SakuraioController < ApplicationController
  skip_before_action :authenticate
  before_action :set_device
  before_action :set_webhook_token

  def webhook
    case params[:type]
    when 'channels'
      handle_channels_message
    end

    head :ok
  end

  def file
    # TODO: suppport OS images
    device_id = @device.device_id
    render plain: "#{MakeStack.settings[:protocol]}://#{MakeStack.settings[:host]}:#{MakeStack.settings[:port]}/api/v1/images/app/#{device_id}/latest"
  end

  private

  def handle_channels_message
    channels = []
    params.dig(:payload, :channels).each do |entry|
      channels[entry[:channel].to_i] = entry[:value]
    end

    # ["85a57374617465a7", "626f6f74696e67a9", ...] => "\x85\xA5\x73\x74..."
    payload = channels.join.scan(/../).map(&:hex).pack('c*')
    SMMSService.receive(payload, device_id: @device.device_id)

    resp = SMMSService.payload_for(@device)
    PushToSakuraioJob.perform_now(
      webhook_token: @webhook_token,
      module_id: @device.sakuraio_module_token,
      payload: resp
    )
  end

  def set_device
    @device ||= Device.find_by_sakuraio_module_token!(params[:module])
  end

  def set_webhook_token
    unless @device.app
      logger.warn "device is not associated to any apps"
      head :accepted
      return false
    end

    @integration ||= Integration.where(service: 'sakuraio', app: @device.app).first
    unless @integration
      logger.warn "no sakura.io integration in the app"
      head :accepted
      return false
    end

    @webhook_token ||= JSON.parse(@integration.config)['incoming_webhook_token']
  end
end
