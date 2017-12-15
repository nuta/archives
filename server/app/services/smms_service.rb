#
#  TODO: needs refactoring and tests
#
module SMMSService
  extend self

  SMMS_VERSION = 1
  SMMS_HMAC_MSG = 0x06
  SMMS_TIMESTAMP_MSG = 0x07
  SMMS_DEVICE_ID_MSG = 0x0a
  SMMS_DEVICE_INFO_MSG = 0x0b
  SMMS_LOG_MSG = 0x0c
  SMMS_OS_VERSION_MSG = 0x10
  SMMS_APP_VERSION_MSG = 0x11
  SMMS_OS_IMAGE_HMAC_MSG = 0x12
  SMMS_APP_IMAGE_HMAC_MSG = 0x13
  SMMS_STORE_MSG = 0x20

  #
  #  Device -> Server
  #
  def receive(payload, hmac_enabled: true, device_id: nil)
    # Be careful! The message HMAC is not verified yet.
    messages, hmac_protected_end = parse_payload(payload, defaults: { device_id: device_id })

    #
    #  Verify messages.
    #
    unless messages[:device_id]
      raise ActionController::BadRequest.new(), "`device_id' is not set"
    end

    device = Device.authenticate(messages[:device_id])
    unless device
      raise ActiveRecord::RecordNotFound
    end

    #  Verify the message timestamp.
    if hmac_enabled
      unless messages[:hmac]
        raise ActionController::BadRequest.new(), "`hmac' is not set"
      end

      unless messages[:timestamp]
        raise ActionController::BadRequest.new(), "`timestamp' is not set"
      end

      if DateTime.now.utc - DateTime.parse(messages[:timestamp]) > 3.minutes
        raise ActionController::BadRequest.new(), "timestamp is too old"
      end

      # Verify the message hmac.
      if messages[:hmac] != device.sign(payload[0..hmac_protected_end])
        raise ActionController::BadRequest.new(), "invalid HMAC digest"
      end
    end

    #
    #  Verified the message. It's now safe to update DB.
    #
    device.last_heartbeated_at = Time.now

    if messages[:device_info]
      device_info = messages[:device_info].unpack('C')[0]
      state = %w[_ booting ready running][device_info & 0x07]
      os = %w[_ linux][(device_info >> 4) & 0x03]
      debug_mode = (device_info & 0x08) != 0

      unless Device::DEVICE_STATES.include?(state)
        raise ActionController::BadRequest.new(), "invalid `device_info.state'"
      end

      unless Device::SUPPORTED_OS.include?(os)
        raise ActionController::BadRequest.new(), "invalid `device_info.os'"
      end

      device.status = state
      device.debug_mode = debug_mode
      device.current_os = os
    end

    if messages[:os_version]
      device.current_os_version = messages[:os_version]
    end

    if messages[:app_version]
      device.current_app_version = messages[:app_version]
    end

    if messages[:log]
      device.append_log(messages[:log])
    end

    device
  end

  #
  #  Device <- Server
  #
  def payload_for(device, include_hmac: true)
    payload = ""
    deployment = device.latest_deployment
    app_version = deployment.try(:version).try(:to_s)
    app_os_version = device.try(:app).try(:os_version)

    if app_os_version && device.current_os_version.value != 'dev' && device.current_os_version.value != app_os_version
      payload += generate_message(SMMS_OS_VERSION_MSG, app_os_version)
      if include_hmac
        os_image_shasum = MakeStack.os_releases.dig(app_os_version, device.current_os.value,
          :assets, device.device_type, :shasum)

        unless os_image_shasum
          raise "BUG: os image not found in MakeStack.os_releases."
        end

        os_image_hmac = device.sign(os_image_shasum)
        payload += generate_message(SMMS_OS_IMAGE_HMAC_MSG, os_image_hmac)
      end
    end

    if app_version && device.current_app_version.value != app_version
      payload += generate_message(SMMS_APP_VERSION_MSG, app_version)
      if include_hmac
        app_image_hmac = device.sign(deployment.image_shasum)
        payload += generate_message(SMMS_APP_IMAGE_HMAC_MSG, app_image_hmac)
      end
    end

    # config
    device.formatted_configs.each_with_index do |(key, config), index|
      data = generate_variable_length(key) + key + config[:value]
      payload += generate_message(SMMS_STORE_MSG, data)
    end

    if include_hmac
      payload += generate_message(SMMS_TIMESTAMP_MSG, Time.now.utc.iso8601)
      hmacMsgLength = 1 + 1 + 64 # type, length, sha256sum

      dummy = payload + 'a' * hmacMsgLength
      header = [SMMS_VERSION << 4].pack('C') + generate_variable_length(dummy)

      hmac = device.sign(header + payload)
      payload += generate_message(SMMS_HMAC_MSG, hmac)
    else
      header = [SMMS_VERSION << 4].pack('C') + generate_variable_length(payload)
    end

    header + payload
  end

  private

  def parse_payload(payload, defaults: {})
    if payload.length < 2
      raise ActionController::BadRequest.new(), 'too short smms payload'
    end

    #
    #  Parse the header.
    #
    version = payload.unpack('C').first >> 4
    if version != 1
      raise ActionController::BadRequest.new(), "unsupported API version"
    end

    total_length, total_length_length = parse_variable_length(payload[1..-1])
    header_length = 1 + total_length_length

    #
    #  Parse messages.
    #
    messages = {}
    defaults.each do |key, value|
      messages[key] ||= value
    end

    hmac_protected_end = nil
    offset = header_length
    while offset < header_length + total_length
      if hmac_protected_end
        raise 'invalid payload: hmac message must be the last one'
      end

      type = payload[offset].unpack('C')[0]
      length, length_length = parse_variable_length(payload[(offset + 1)..-1])
      data = payload[offset + 1 + length_length, length]

      case type
      when SMMS_HMAC_MSG
        hmac_protected_end = offset - 1
        messages[:hmac] = data
      when SMMS_TIMESTAMP_MSG
        messages[:timestamp] = data
      when SMMS_DEVICE_ID_MSG
        messages[:device_id] = data
      when SMMS_DEVICE_INFO_MSG
        messages[:device_info] = data
      when SMMS_LOG_MSG
        messages[:log] = data
      when SMMS_OS_VERSION_MSG
        messages[:os_version] = data
      when SMMS_APP_VERSION_MSG
        messages[:app_version] = data
      when SMMS_STORE_MSG
        key_length, key_length_length = parse_variable_length(data)
        key = data[key_length_length,key_length]
        value = data[(key_length_length + key_length)..-1]
        messages[:configs] ||= []
        messages[:configs] << { key: key, value: value }
      end

      offset += 1 + length_length + length
    end

    [messages, hmac_protected_end]
  end

  def generate_message(type, data)
    [type].pack('C') + generate_variable_length(data) + data
  end

  def generate_variable_length(buf)
    len = buf.length
    lenbuf = ""

    while len > 0
      digit = len % 0x80
      len /= 0x80
      lenbuf += [((len > 0) ? 0x80 : 0) | digit].pack('C')
    end

    (lenbuf.length > 0) ? lenbuf : [0x00].pack('C')
  end

  def parse_variable_length(buf)
    length = 0
    i = 0
    base = 1
    while true
      if i == buf.length
        raise 'invalid variable length'
      end

      byte = buf[i].unpack('C')[0]
      length += (byte & 0x7f) * base

      if byte & 0x80 == 0
        return [length, i + 1]
      end

      i += 1
      base *= 128
    end
  end
end
