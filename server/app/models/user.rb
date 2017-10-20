class User < ActiveRecord::Base
  devise :database_authenticatable, :registerable,
          :recoverable, :rememberable, :trackable, :validatable,
          :confirmable

  include DeviseTokenAuth::Concerns::User

  has_many :apps, dependent: :destroy
  has_many :devices, dependent: :destroy

  # Quotas
  APPS_MAX_NUM = 30
  DEVICES_MAX_NUM = 30
  SOURCE_FILES_MAX_NUM_PER_APP = 16

  RESERVED_USER_NAMES = %w(
    home app apps devise devices settings builds deployments
    admin user users team teams create me account admin cloud
    api blog cache changelog enterprise help jobs lists login logout news
    plans popular projects security search register invite shop jobs
    translations signup status tour wiki better auth documentation
    support easter-egg)

  validates_presence_of   :username
  validates_uniqueness_of :username, case_sensitive: false
  validates_exclusion_of  :username, in: RESERVED_USER_NAMES,
                          message: "%{value} is not available."
  validates_format_of     :username, with: /\A[a-zA-Z][a-zA-Z0-9\_\-]+\z/
  validates_length_of     :username, in: 3..64

  before_save do
    skip_confirmation! if Rails.env.development?
  end
end
