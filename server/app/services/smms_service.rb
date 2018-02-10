#
#  TODO: needs refactoring and tests
#
module SMMSService
  extend self

  SMMS_VERSION = 1
  SMMS_HMAC_MSG      = 0x01
  SMMS_CACHE_MSG     = 0x02
  SMMS_DEVICE_ID_MSG = 0x03
  SMMS_LOG_MSG       = 0x04
  SMMS_COMMAND_MSG   = 0x05
  SMMS_GET_MSG       = 0x06
  SMMS_OBSERVE_MSG   = 0x07
  SMMS_REPORT_MSG    = 0x08
  SMMS_CONFIG_MSG    = 0x09
  SMMS_UPDATE_MSG    = 0x0a
  SMMS_OSUPDATE_MSG  = 0x0b
  SMMS_CURRENT_VERSION_REPORT = 0x0001

  #
  #  Device -> Server
  #
  def receive(payload, device_id: nil)
    messages = parse_payload(payload, defaults: { device_id: device_id })
    device_id = messages[:device_id]
    log = messages[:log]
    reports = messages[:reports]

    unless device_id
      raise ActionController::BadRequest.new(), "`device_id' is not set"
    end

    device = Device.authenticate(device_id)
    unless device
      raise ActiveRecord::RecordNotFound
    end

    # Verified the message. It's now safe to update DB.
    device.last_heartbeated_at = Time.now

    if reports[:app_version]
      device.current_app_version = reports[:app_version]
    end

    if log
      device.append_log(log)
    end

    device
  end

  #
  #  Device <- Server
  #
  def payload_for(device)
    payload = ""
    deployment = device.latest_deployment
    app_version = deployment.try(:version).try(:to_i)

    if app_version && device.current_app_version.value != app_version
      data = [
        2, # Download from adapter-specific way.
        app_version
      ].pack('CN')
      payload += generate_message(SMMS_UPDATE_MSG, data)
    end

    # config
    device.formatted_configs.each_with_index do |(key, config), index|
      data = generate_variable_length(key) + key + config[:value]
      payload += generate_message(SMMS_CONFIG_MSG, data)
    end

    header = [SMMS_VERSION << 4].pack('C') + generate_variable_length(payload)
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
    messages = { reports: {} }
    defaults.each do |key, value|
      messages[key] ||= value
    end

    offset = header_length
    while offset < header_length + total_length
      type = payload[offset].unpack('C')[0]
      length, length_length = parse_variable_length(payload[(offset + 1)..-1])
      data = payload[offset + 1 + length_length, length]

      case type
        when SMMS_DEVICE_ID_MSG
          messages[:device_id] = data
        when SMMS_LOG_MSG
          messages[:log] = data
        when SMMS_REPORT_MSG
          id, value = data.unpack('nN')
          case id
        when SMMS_CURRENT_VERSION_REPORT
          messages[:reports][:app_version] = value
        end
      end

      offset += 1 + length_length + length
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
end
