class DeviceMapping < ApplicationRecord
  belongs_to :device

  SUPPORTED_TYPES = %w(sakuraio)

  validates :token, uniqueness: true
  validates :type, inclusion: { in: SUPPORTED_TYPES }
end
