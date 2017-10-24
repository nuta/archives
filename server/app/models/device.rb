class Device < ApplicationRecord
  belongs_to :user
  belongs_to :app, optional: true
  has_many   :device_stores, dependent: :destroy
  has_many   :device_mappings, dependent: :destroy

  value :status, expiration: 45.minutes
  value :debug_mode, expiration: 45.minutes
  value :last_heartbeated_at, expiration: 3.days
  value :current_os_version, expiration: 45.minutes
  value :current_app_version, expiration: 45.minutes
  sorted_set :log, expiration: 1.hours

  SUPPORTED_TYPES = %w(mock raspberrypi3)
  DEVICE_STATES = %i(new booting ready running relaunch reboot down)
  RESERVED_DEVICE_NAMES = %w(new)
  DEVICE_NAME_REGEX = /\A[a-zA-Z][a-zA-Z0-9\-\_]*\z/
  DEVICE_ID_LEN = 40
  DEVICE_ID_REGEX = /\A[a-zA-Z0-9\-\_\.\~]{#{DEVICE_ID_LEN}}\z/
  DEVICE_ID_PREFIX_LEN = 20
  DEVICE_SECRET_LEN = 40
  DEVICE_SECRET_REGEX = /\A[a-zA-Z0-9\-\_\.\~]{#{DEVICE_SECRET_LEN}}\z/
  TAG_NAME_REGEX = /\A[a-zA-Z][a-zA-Z0-9\:\/\-\_]*\z/
  TAG_LEN = 128
  DEVICE_LOG_MAX_LINES = 512

  validates_presence_of   :user
  validates_presence_of   :device_id
  validates_uniqueness_of :device_id
  validates_format_of     :device_id, with: DEVICE_ID_REGEX
  validates_presence_of   :device_id_prefix
  validates_uniqueness_of :device_id_prefix
  validates_uniqueness_of :name, scope: :user_id, case_sensitive: false
  validates_exclusion_of  :name, in: RESERVED_DEVICE_NAMES,
                          message: "%{value} is not available."
  validates_length_of     :name, in: 0..128
  validates_format_of     :name, with: DEVICE_NAME_REGEX
  validates_inclusion_of  :device_type, in: SUPPORTED_TYPES
  validates_format_of     :tag, with: TAG_NAME_REGEX, allow_nil: true
  validates_length_of     :tag, in: 0..TAG_LEN, allow_nil: true
  validate :device_id_prefix_is_prefix
  validate :validate_num_of_devices, on: :create

  def device_id_prefix_is_prefix
    unless self.device_id.start_with?(self.device_id_prefix)
      errors.add(:device_id_prefix, "is not prefix of device_id (BUG).")
    end
  end

  def validate_num_of_devices
    if user && user.devices.count >= User::DEVICES_MAX_NUM
      errors.add(:devices, "are too many.")
    end
  end

  def stores
    stores = {}

    AppStore.where(app: self.app).find_each do |store|
      stores[store.key] = {
         value: store.value,
          scope: 'app',
          override: false
         }
    end

    DeviceStore.where(device: self).find_each do |store|
      stores[store.key] = {
        value: store.value,
        scope: 'device',
        override: stores.key?(store.key)
      }
    end

    stores
  end

  def reset_credentials
    device_id     = RandomIdGenerator::generate(DEVICE_ID_LEN)
    device_secret = RandomIdGenerator::generate(DEVICE_SECRET_LEN)

    self.device_id = device_id
    self.device_id_prefix = device_id[0, DEVICE_ID_PREFIX_LEN]
    self.device_secret = device_secret
  end

  def get_log(since = 0)
    _lines = self.log.rangebyscore(since.to_f, Float::INFINITY) || []
    lines = _lines.join("\n").scrub("?").split("\n").map() do |l|
      c = l.split(":", 4)
      { time: c[0], index: c[1], device_name: c[2], body: c[3] }
    end

    lines
  end

  def append_log_to(target, device, lines, time)
    if target == :app
      unless device.app
        # The device is not associated with any app. Aborting.
        return
      end

      log = app.log
      max_lines = App::APP_LOG_MAX_LINES
      integrations = device.app.integrations.all
    else
      log = device.log
      max_lines = DEVICE_LOG_MAX_LINES
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

  def deployment
    @deployment ||= Deployment \
                      .where(app: self.app, tag: [self.tag, nil])
                      .order("created_at")
                      .last
  end

  def app_image(version)
    deployment.try(:image)
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
