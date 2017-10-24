class Deployment < ApplicationRecord
  belongs_to :app

  MAX_IMAGE_SIZE      = 256 * 1024
  MAX_DEBUG_DATA_SIZE = 32 * 1024

  validates_presence_of :app
  validates :debug, length: { in: 0..MAX_DEBUG_DATA_SIZE }, allow_nil: true
  validates_uniqueness_of :version, scope: :app_id
  validates :tag, format: { with: Device::TAG_NAME_REGEX },
                  length: { in: 0..Device::TAG_LEN }, allow_nil: true
  validates :comment, length: { in: 0..1024 }, allow_nil: true
  validate :validate_image_format
  validate :validate_image_size

  before_create :set_version

  def initialize(attrs = {})
    image_file = attrs.delete(:image)
    super

    if image_file.instance_of?(ActionDispatch::Http::UploadedFile)
      self.image = image_file.read
    else
      self.image = image_file
    end
  end

  def validate_image_format
    unless self.image[0..1] == 'PK'
      errors.add(:image, 'is not valid zip file.')
    end
  end

  def validate_image_size
    self.image.size < MAX_IMAGE_SIZE
  end

  def set_version
    self.version = (Deployment.where(app: self.app).maximum(:version) || 0) + 1
  end
end
