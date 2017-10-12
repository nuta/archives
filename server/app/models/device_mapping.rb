class DeviceMapping < ApplicationRecord
  belongs_to :device

  SUPPORTED_TYPES = %w(sakuraio)
  
  validates :token, uniqueness: { scope: :device_id }
  validates :type, inclusion: { in: SUPPORTED_TYPES }
end
