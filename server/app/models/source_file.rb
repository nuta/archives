class SourceFile < ApplicationRecord
  belongs_to :app

  PATH_MAX_LEN = 128
  PATH_REGEX = /\A[a-zA-Z0-9\-\_\.]+\z/
  BODY_MAX_LEN = 8 * 1024

  validates_presence_of   :app
  validates_presence_of   :path
  validates_uniqueness_of :path, scope: :app_id, case_sensitive: false
  validates_length_of     :path, in: 1..PATH_MAX_LEN
  validates_format_of     :path, with: PATH_REGEX
  validates :body, length: { in: 0..BODY_MAX_LEN}, allow_nil: false
  validate :validate_num_of_files, on: :create

  def validate_num_of_files
    if app && app.source_files.count >= User::SOURCE_FILES_MAX_NUM_PER_APP
      errors.add(:source_files, "are too many.")
    end
  end
end
