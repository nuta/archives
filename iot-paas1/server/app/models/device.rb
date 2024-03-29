class Device < ApplicationRecord
  include Quota
  include DeviceLog

  belongs_to :user
  belongs_to :app, optional: true
  has_many :configs, as: :owner, dependent: :destroy

  value :status, expiration: 5.minutes
  value :debug_mode, expiration: 45.minutes
  value :last_heartbeated_at, expiration: 3.days
  value :current_os, expiration: 45.minutes
  value :current_os_version, expiration: 45.minutes
  value :current_app_version, expiration: 45.minutes
  value :last_command_id, expiration: 3.hours
  list :pending_commands, marshal: true, expiration: 15.minutes
  hash_key :command_results, expiration: 5.minutes

  SUPPORTED_OS = %w(linux)
  SUPPORTED_TYPES = %w(sdk raspberrypi3 esp32)
  DEVICE_STATES = %w(new booting ready running relaunch reboot down)
  RESERVED_DEVICE_NAMES = %w(new)
  DEVICE_NAME_REGEX = /\A[a-zA-Z][a-zA-Z0-9\-\_]*\z/
  DEVICE_ID_LEN = 40
  DEVICE_ID_REGEX = /\A[a-zA-Z0-9\-\_\.\~]{#{DEVICE_ID_LEN}}\z/
  DEVICE_ID_PREFIX_LEN = 20
  DEVICE_SECRET_LEN = 40
  DEVICE_SECRET_REGEX = /\A[a-zA-Z0-9\-\_\.\~]{#{DEVICE_SECRET_LEN}}\z/
  TAG_NAME_REGEX = /\A[a-zA-Z][a-zA-Z0-9\:\/\-\_]*\z/
  TAG_LEN = 128
  LOG_MAX_LINE_LENGTH = 128
  LOG_MAX_LINES = 256

  delegate :id, to: :app, prefix: true, allow_nil: true
  quota scope: :user_id, limit: User::DEVICES_MAX_NUM

  validates :device_id,
    uniqueness: :device_id,
    format: { with: DEVICE_ID_REGEX }

  validate :device_id_prefix_is_prefix
  validates :device_id_prefix,
    uniqueness: :device_id_prefix

  validates :device_secret,
    uniqueness: true,
    format: { with: DEVICE_SECRET_REGEX }

  validates :name,
    uniqueness: { scope: :user_id, case_sensitive: false },
    exclusion: { in: RESERVED_DEVICE_NAMES, message: "%{value} is not available." },
    length: { in: 0..128 },
    format: { with: DEVICE_NAME_REGEX }

  validates :tag,
   format: { with: TAG_NAME_REGEX },
   length: { in: 0..TAG_LEN },
   allow_nil: true

  validates :device_type,
    inclusion: { in: SUPPORTED_TYPES }

  after_initialize :set_credentials, if: :new_record?

  def set_credentials
    self.reset_credentials
  end

  def device_id_prefix_is_prefix
    unless self.device_id
      errors.add(:device_id, "is not set (BUG).")
      return
    end

    unless self.device_id_prefix
      errors.add(:device_id_prefix, "is not set (BUG).")
      return
    end

    unless self.device_id.start_with?(self.device_id_prefix)
      errors.add(:device_id_prefix, "is not prefix of device_id (BUG).")
      return
    end
  end

  def formatted_configs
    configs = {}

    Config.where(owner_type: 'App', owner_id: self.app_id).find_each do |config|
      configs[config.key] = { value: config.value }
    end

    Config.where(owner_type: 'Device', owner_id: self.id).find_each do |config|
      configs[config.key] = { value: config.value }

      if config.is_command?
        config.destroy
      end
    end

    Hash[configs.sort]
  end

  def formatted_command_results
    results = {}
    self.command_results.value.map do |k, v|
      results[k] = v
    end

    results
  end

  def reset_credentials
    device_id     = RandomIdGenerator::generate(DEVICE_ID_LEN)
    device_secret = RandomIdGenerator::generate(DEVICE_SECRET_LEN)

    self.device_id = device_id
    self.device_id_prefix = device_id[0, DEVICE_ID_PREFIX_LEN]
    self.device_secret = device_secret
  end

  def sign(data)
    OpenSSL::HMAC.hexdigest('SHA256', self.device_secret, data)
  end

  def latest_deployment
    Deployment \
      .where(app: self.app, tag: [self.tag, nil])
      .order("created_at")
      .last
  end

  def app_image(version)
    return nil unless self.app

    if version == 'latest'
      deployment = self.latest_deployment
    else
      deployment = Deployment
        .where(app: self.app, tag: [self.tag, nil])
        .find_by_version(version)
    end

    deployment.try(:image)
  end

  def disassociate!
    self.update_attributes!(app: nil)
  end

  def invoke_command!(command, arg)
    command_id = (self.last_command_id.value.to_i || 0) + 1
    pending_commands << { id: command_id, key: command, arg: arg }
    self.last_command_id = command_id
  end

  def append_log_to(log, lines, default_time, integrations)
    device_name = self.name
    lines.each_with_index do |line, index|
      if line.length > LOG_MAX_LINE_LENGTH
        line = line[0..(LOG_MAX_LINE_LENGTH)]  + '...'
      end

      # Date in unixtime.
      if line.start_with?("=")
        unixtime, line = line.split(' ', 2)
        time = unixtime.gsub('=', '').to_i.to_f
      else
        time = default_time
      end

      log["#{time}:#{index}:#{device_name}:#{line}"] = time

      m = /\A@(?<event>[^ ]+) (?<body>.*)\z/.match(line)
      if m
        # Detected a event published from the device.
        HookService.invoke(integrations, :event_published, self, {
          event: m['event'],
          body: m['body']
        })
      end

      m = /\A>(?<command_id>[^ ]+) (?<return_value>.*)\z/.match(line)
      if m
        # Detected a event published from the device.
        self.command_results[m['command_id']] = m['return_value']

        HookService.invoke(integrations, :command_invoked, self, {
          command_id: m['command_id'],
          return_value: m['return_value']
        })
      end
    end

    log.remrangebyrank(0, -LOG_MAX_LINES)
  end

  def append_to_device_log(lines, time)
    append_log_to(self.log, lines, time, [])
  end

  def append_to_app_log(lines, time)
    app = self.app
    if app
      integrations = app.integrations.all
      log = app.log
      append_log_to(log, lines, time, integrations)
    end
  end

  def append_log(body)
    return unless body.is_a?(String)
    lines = body.split("\n").reject(&:empty?)
    time = Time.now.to_f
    append_to_device_log(lines, time)
    append_to_app_log(lines, time)
  end

  def self.authenticate(device_id)
    prefix = device_id[0, DEVICE_ID_PREFIX_LEN]
    device = Device.find_by_device_id_prefix(prefix)

    if device && ActiveSupport::SecurityUtils.secure_compare(device.device_id, device_id)
      device
    else
      nil
    end
  end
end
