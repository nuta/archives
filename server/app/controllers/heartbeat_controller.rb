class HeartbeatController < ApplicationController
  skip_before_action :authenticate
  before_action :set_messages, only: [:heartbeat]

  def heartbeat
    @device = Device.authenticate(@messages[:device_id])
    unless @device
      head :forbidden
      return
    end

    @device.append_log(@messages[:log])
    @device.update_attributes!(
      status: @messages.dig(:device_info, :state),
      last_heartbeated_at: Time.now
    )

    render body: heartbeat_response
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

  private

  SMMS_VERSION = 0x01
  SMMS_DEVICE_ID = 0x0a
  SMMS_DEVICE_INFO = 0x0b
  SMMS_LOG = 0x0c
  SMMS_APP_VERSION = 0x0d
  SMMS_APP_UPDATE_REQUEST = 0x11
  SMMS_STORE = 0x40
  SMMS_STORE_END = 0x7f
  SMMS_STORE_NUM = SMMS_STORE_END - SMMS_STORE + 1

  def set_messages
    data = MessagePack.unpack(request.body.read)
    states = [:new, :booting, :ready, :down, :reboot, :relaunch]

    device_info = {
      state: states[(data[SMMS_DEVICE_INFO] || 0) & 0x07]
    }

    @messages ||= {
      version: data[SMMS_VERSION],
      device_info: device_info,
      device_id: data[SMMS_DEVICE_ID],
      log: data[SMMS_LOG],
      app_version: data[SMMS_APP_VERSION]
    }

    if @messages[:version] != 1
      head :not_acceptable
      return
    end

    unless @messages.key?(:device_id)
      head :bad_request
      return
    end
  end

  def heartbeat_response
    payload = {}
    deployment = @device.deployment

    if deployment && @messages[:version] != deployment.version
      # TODO: os_update_request
  
      payload[SMMS_APP_UPDATE_REQUEST] = deployment.version
    end
  
    # store
    @device.stores.each_with_index do |(key, store), index|
      if index >= SMMS_STORE_NUM
        logger.warn "too many stores"
        break
      end
  
      payload[SMMS_STORE + index] = [key, store[:value]]
    end
  
    payload.to_msgpack
  end
 end
