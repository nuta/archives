class Device < ApplicationRecord
  include Quota
  include DeviceLog

  belongs_to :user
  belongs_to :app, optional: true
  has_many :stores, as: :owner, dependent: :destroy

  value :status, expiration: 45.minutes
  value :debug_mode, expiration: 45.minutes
  value :last_heartbeated_at, expiration: 3.days
  value :current_os_version, expiration: 45.minutes
  value :current_app_version, expiration: 45.minutes
  value :last_command_id, expiration: 3.hours
  hash_key :command_results, expiration: 5.minutes

  SUPPORTED_TYPES = %w(sdk raspberrypi3)
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

  def formatted_stores
    stores = {}

    Store.where(owner_type: 'App', owner_id: self.app_id).find_each do |store|
      stores[store.key] = { value: store.value }
    end

    Store.where(owner_type: 'Device', owner_id: self.id).find_each do |store|
      stores[store.key] = { value: store.value }

      if store.is_command?
        store.destroy
      end
    end

    Hash[stores.sort]
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
    Store.create!(
      owner_type: 'Device',
      owner_id: self.id,
      key: "<#{command_id} #{command}",
      data_type: 'string',
      value: arg
    )

    self.last_command_id = command_id
  end

  def append_log_to(target, device, lines, time)
    max_lines = APP_LOG_MAX_LINES
    if target == :app
      unless device.app
        # The device is not associated with any app. Aborting.
        return
      end

      log = app.log
      integrations = device.app.integrations.all
    else
      log = device.log
      integrations = []
    end

    device_name = self.name
    lines.each_with_index do |line, index|
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

    log.remrangebyrank(0, -max_lines)
  end

  def append_log(body)
    return unless body.is_a?(String)

    device_name = self.name
    time = Time.now.to_f
    lines = body.split("\n").reject(&:empty?)

    append_log_to(:device, self, lines, time)
    append_log_to(:app, self, lines, time)
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
