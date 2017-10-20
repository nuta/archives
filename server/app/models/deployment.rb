class Deployment < ApplicationRecord
  belongs_to :app

  MAX_IMAGE_SIZE      = 32 * 1024
  MAX_DEBUG_DATA_SIZE = 32 * 1024
  
  validates_presence_of :app
  validates :debug, length: { in: 0..MAX_DEBUG_DATA_SIZE }, allow_nil: true
  validates_uniqueness_of :version, scope: :app_id
  validates :tag, format: { with: Device::TAG_NAME_REGEX },
                  length: { in: 0..Device::TAG_LEN }, allow_nil: true
  validates :comment, length: { in: 0..1024 }, allow_nil: true
  validate :validate_image_size

  before_create :set_version

  def validate_image_size
    self.image.size < MAX_IMAGE_SIZE
  end

  def set_version
    self.version = (Deployment.where(app: self.app).maximum(:version) || 0) + 1
  end
end
