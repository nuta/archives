class App < ApplicationRecord
  belongs_to :user
  has_many :deployments, dependent: :destroy
  has_many :app_stores, dependent: :destroy
  has_many :source_files, dependent: :destroy
  has_many :integrations, dependent: :destroy
  has_many :devices # nullified in disassociate_devices on destroy
  before_destroy :disassociate_devices

  sorted_set :log, expiration: 1.hours

  RESERVED_APP_NAMES = %w(new)
  SUPPORTED_APIS = %w(makestack linux)
  APP_LOG_MAX_LINES = 512

  validates_presence_of   :user
  validates_uniqueness_of :name, scope: :user_id, case_sensitive: false
  validates_exclusion_of  :name, in: RESERVED_APP_NAMES,
                          message: "%{value} is not available."
  validates_format_of     :name, with: /\A[a-zA-Z][a-zA-Z0-9\-\_]*\z/
  validates_length_of     :name, in: 1..128
  validates_inclusion_of  :api, in: SUPPORTED_APIS,
                          message: "%{value} is unsupported (unknown) API."
  validate :validate_num_of_apps, on: :create

  # TODO: validate :os_version

  def validate_num_of_apps
    if user && user.apps.count >= User::APPS_MAX_NUM
      errors.add(:apps, "are too many.")
    end
  end

  def get_log(since = 0)
    _lines = self.log.rangebyscore(since.to_f, Float::INFINITY) || []
    lines = _lines.join("\n").scrub("?").split("\n").map() do |l|
      c = l.split(":", 4)
      { time: c[0], index: c[1], device_name: c[2], body: c[3] }
    end

    lines
  end

  def disassociate_devices
    self.devices.find_each do |device|
      device.disassociate!
    end
  end
end