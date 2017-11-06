class App < ApplicationRecord
  include Quota
  include DeviceLog
  include DefaultValueFor

  belongs_to :user
  has_many :deployments, dependent: :destroy
  has_many :stores, as: :owner, dependent: :destroy
  has_many :source_files, dependent: :destroy
  has_many :integrations, dependent: :destroy
  has_many :devices # nullified in disassociate_devices on destroy
  before_destroy :disassociate_devices

  RESERVED_APP_NAMES = %w(new)
  SUPPORTED_APIS = %w(makestack linux)
  APP_LOG_MAX_LINES = 512

  quota scope: :user_id, limit: User::APPS_MAX_NUM

  validates :name,
    presence: true,
    uniqueness: { scope: :user_id, case_sensitive: false },
    exclusion: { in: RESERVED_APP_NAMES, message: "`%{value}' is not available." },
    format: { with: /\A[a-zA-Z][a-zA-Z0-9\-\_]*\z/ },
    length: { in: 1..128 }

  validates :api,
    inclusion: { in: SUPPORTED_APIS, message: "`%{value}' is unsupported (unknown) API." }

  validate :validate_os_version

  default_value_for :os_version, value: MakeStack.os_releases.keys[-1]

  def validate_os_version
    unless MakeStack.os_releases.include?(self.os_version)
      errors.add(:os_version, 'is not released.')
    end
  end

  def disassociate_devices
    self.devices.find_each do |device|
      device.disassociate!
    end
  end
end
