class DeviceMapping < ApplicationRecord
  belongs_to :device

  SUPPORTED_TYPES = %w(sakuraio)

  validates :token, uniqueness: true
  validates :token_type, inclusion: { in: SUPPORTED_TYPES }
end
