class SourceFile < ApplicationRecord
  include Quota

  belongs_to :app

  PATH_MAX_LEN = 128
  PATH_REGEX = /\A[a-zA-Z0-9\-\_\.]+\z/
  BODY_MAX_LEN = 8 * 1024

  quota scope: :app_id, limit: User::SOURCE_FILES_MAX_NUM_PER_APP

  validates :path,
    uniqueness: { scope: :app_id, case_sensitive: false },
    length: { in: 1..PATH_MAX_LEN },
    format: { with: PATH_REGEX }

  validates :body,
    length: { in: 0..BODY_MAX_LEN}
end
