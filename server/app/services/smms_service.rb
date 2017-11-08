module SMMSService
  extend self

  SMMS_VERSION = 1
  SMMS_DEVICE_ID_MSG = 0x0a
  SMMS_DEVICE_INFO_MSG = 0x0b
  SMMS_LOG_MSG = 0x0c
  SMMS_OS_VERSION_MSG = 0x10
  SMMS_APP_VERSION_MSG = 0x11
  SMMS_APP_IMAGE_HMAC_MSG = 0x13
  SMMS_STORE_MSG = 0x20

  #
  #  Device -> Server
  #
  def receive(payload, hmac_enabled: true, timestamp: nil, hmac: nil, device_id: nil)
    # Be careful! The message HMAC is not verified yet.
    messages = parse_payload(payload, defaults: {
      device_id: device_id,
      hmac: hmac,
      timestamp: timestamp
    })

    #
    #  Verify messages.
    #
    device = Device.authenticate(messages[:device_id])
    unless device
      raise ActiveRecord::RecordNotFound
    end

    #  Verify the message timestamp.
    if hmac_enabled
      unless messages[:timestamp]
        raise ActionController::BadRequest.new(), "`timestamp' is not set"
      end

      if DateTime.now.utc - DateTime.parse(messages[:timestamp]) > 3.minutes
        raise ActionController::BadRequest.new(), "timestamp is too old"
      end

      # Verify the message hmac.
      if messages[:hmac] != computeHMAC(device.device_secret, messages[:timestamp], payload)
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
      debug_mode = (device_info & 0x08) != 0

      unless Device::DEVICE_STATES.include?(state)
        raise ActionController::BadRequest.new(), "invalid `device_info.state'"
      end

      device.status = state
      device.debug_mode = debug_mode
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
  def payload_for(device)
    payload = ""
    deployment = device.deployment
    app_version = deployment.try(:version).try(:to_s)
    app_os_version = device.try(:app).try(:os_version)

    if app_os_version && device.current_os_version.value != app_os_version
      payload += generate_message(SMMS_OS_VERSION_MSG, app_os_version)
    end

    if app_version && device.current_app_version.value != app_version
      app_image_hmac = device.sign(deployment.image_shasum)
      payload += generate_message(SMMS_APP_VERSION_MSG, app_version)
      payload += generate_message(SMMS_APP_IMAGE_HMAC_MSG, app_image_hmac)
    end

    # store
    device.formatted_stores.each_with_index do |(key, store), index|
      data = generate_variable_length(key).first + key + store[:value]
      payload += generate_message(SMMS_STORE_MSG, data)
    end

    header = [SMMS_VERSION << 4].pack('C') + generate_variable_length(payload)
    header + payload
  end

  #
  #  Computes a payload HMAC signature for the device.
  #
  def sign(device, payload)
    timestamp = Time.now.utc.iso8601
    return timestamp, computeHMAC(device.device_secret, timestamp, payload)
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
    offset = header_length
    while offset < header_length + total_length
      type = payload[offset].unpack('C')[0]
      length, length_length = parse_variable_length(payload[(offset + 1)..-1])
      data = payload[offset + 1 + length_length, length]

      case type
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
        messages[:stores] ||= []
        messages[:stores] << { key: key, value: value }
      end

      offset += 1 + length_length + length
    end

    defaults.each do |key, value|
      messages[key] ||= value
    end

    messages
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
