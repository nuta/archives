class SakuraioController < ApplicationController
  skip_before_action :authenticate
  before_action :set_device

  def invoke
    case params[:type]
    when 'channels'
      handle_channels_message
    end
  end

  private

  def handle_channels_message
    channels = []
    params.dig(:payload, :channels).each do |entry|
      channels[entry[:channel]] = entry[:value]
    end

    payload = channels.flatten.join
    SMMSService.receive(payload)
  end

  def set_device
    @mapping ||= DeviceMapping.where(type: 'sakuraio', token: params[:module]).first!
    @device ||= @mapping.device
  end
end
