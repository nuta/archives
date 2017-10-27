module SMMSService
  extend self

  SMMS_VERSION = 0x01
  SMMS_DEVICE_ID = 0x0a
  SMMS_DEVICE_INFO = 0x0b
  SMMS_LOG = 0x0c
  SMMS_OS_VERSION = 0x10
  SMMS_APP_VERSION = 0x11
  SMMS_OS_UPDATE_REQUEST = 0x20
  SMMS_APP_UPDATE_REQUEST = 0x21
  SMMS_STORE = 0x40
  SMMS_STORE_END = 0x7f
  SMMS_STORE_NUM = SMMS_STORE_END - SMMS_STORE + 1

  def receive(payload, timestamp, hmac)
    # Be careful! The message HMAC is not verified yet.
    data = MessagePack.unpack(payload)
    states = [:new, :booting, :ready, :down, :reboot, :relaunch]

    # XXX: msgpack-lite used in Node stringifies number keys.
    data.keys.each do |k|
      data[k.to_i] = data[k]
    end

    device_info = {
      state: states[(data[SMMS_DEVICE_INFO] || 0) & 0x07],
      debug_mode: states[(data[SMMS_DEVICE_INFO] || 0) & 0x08] != 0
    }

    messages = {
      version: data[SMMS_VERSION],
      device_info: device_info,
      device_id: data[SMMS_DEVICE_ID],
      log: data[SMMS_LOG],
      os_version: data[SMMS_OS_VERSION],
      app_version: data[SMMS_APP_VERSION]
    }

    if messages[:version] != 1
      raise ActionController::BadRequest.new(), "unsupported API version"
    end

    unless messages.key?(:device_id)
      raise ActionController::BadRequest.new(), "`device_id' is not set"
    end

    unless messages[:os_version].is_a?(String)
      raise ActionController::BadRequest.new(), "`os_version' is not string"
    end

    unless messages[:app_version].is_a?(Integer)
      raise ActionController::BadRequest.new(), "`app_version' is not integer"
    end

    unless Device::DEVICE_STATES.include?(messages.dig(:device_info, :state))
      raise ActionController::BadRequest.new(), "`device_info.state' is not valid"
    end

    device = Device.authenticate(messages[:device_id])
    unless device
      raise ActiveRecord::RecordNotFound
    end

    # Verify the message timestamp.
    if DateTime.now.utc - DateTime.parse(timestamp) > 3.minutes
      raise ActionController::BadRequest.new(), "timestamp is too old"
    end

    # Verify the message hmac.
    if hmac != computeHMAC(device.device_secret, timestamp, payload)
      raise ActionController::BadRequest.new(), "invalid HMAC digest"
    end

    # Verified the message. It's now safe to update DB.
    device.last_heartbeated_at = Time.now
    device.status = messages.dig(:device_info, :state)
    device.debug_mode = messages.dig(:device_info, :debug_mode)
    device.current_os_version = messages[:os_version]
    device.current_app_version = messages[:app_version]
    device.append_log(messages[:log])
    return device
  end

  def payload_for(device)
    payload = {}
    deployment = device.deployment
    app_os_version = device.try(:app).try(:os_version)

    if app_os_version && device.current_os_version.value != app_os_version
      payload[SMMS_OS_UPDATE_REQUEST] = app_os_version
    end

    if deployment && device.current_app_version.value.to_i != deployment.version
      payload[SMMS_APP_UPDATE_REQUEST] = deployment.version
    end

    # store
    device.formatted_stores.each_with_index do |(key, store), index|
      if index >= SMMS_STORE_NUM
        Rails.logger.warn "too many stores"
        break
      end

      payload[SMMS_STORE + index] = [key, store[:value]]
    end

    payload.to_msgpack
  end

  def sign(device, payload)
    timestamp = Time.now.utc.iso8601
    return timestamp, computeHMAC(device.device_secret, timestamp, payload)
  end

  private

  def computeHMAC(device_secret, timestamp, payload)
    sha = Digest::SHA256.new

    if payload.is_a?(File)
      while content = payload.read(4096)
        sha.update(content)
      end
    else
      sha.update(payload)
    end

    return OpenSSL::HMAC.hexdigest('SHA256', device_secret,
                                   timestamp + "\n" + sha.hexdigest)
  end
end
