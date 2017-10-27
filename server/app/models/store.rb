class Store < ApplicationRecord
  include Quota

  belongs_to :owner, polymorphic: true

  # This MUST be smaller than 256 because of a SMMS limitaiton.
  KEY_MAX_LEN = 64

  KEY_REGEX = /\A[a-zA-Z][a-zA-Z0-9\~\!\@\$\%\&\.\-\_]*\z/
  DATA_TYPES = %w(string integer float bool)

  quota scope: :owner_id, limit: User::STORES_MAX_NUM

  validates :key,
    format: { with: KEY_REGEX },
    uniqueness: { scope: [:owner_type, :owner_id] },
    case_sensitive: false,
    length: { in: 1..KEY_MAX_LEN }

  validates :data_type,
    inclusion: { in: DATA_TYPES },
    case_sensitive: false,
    length: { in: 1..64 }

  validates :value,
    length: { in: 0..512 }
end
