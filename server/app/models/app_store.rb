class AppStore < ApplicationRecord
  belongs_to :app

  # This MUST be smaller than 256 because of a MakeStack Protocol limitaitond.
  KEY_MAX_LEN = 64

  KEY_REGEX = /\A[a-zA-Z][a-zA-Z0-9\~\!\@\$\%\&\.\-\_]*\z/
  DATA_TYPES = %w(string integer float bool)

  validates :key, format: { with: KEY_REGEX }, uniqueness: { scope: :app_id },
    case_sensitive: false, length: { in: 1..KEY_MAX_LEN }
  validates :data_type, inclusion: { in: DATA_TYPES },
    case_sensitive: false, uniqueness: { scope: :app_id }, length: { in: 1..64 }
  validates :value, length: { in: 0..512 }

  before_save :set_empty_string_if_value_is_nil

  def set_empty_string_if_value_is_nil
    self.value = "" unless self.value
  end
end
