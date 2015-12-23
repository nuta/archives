require 'digest/sha2'

class User < ActiveRecord::Base
  validates :name, uniqueness: true
  has_many :calendars, dependent: :destroy

  def validate_password(password)
    self.password_hash == hash(password + self.password_salt)
  end

  def password=(password)
    salt = SecureRandom.base64(16)
    self.password_salt = salt
    self.password_hash = hash(password + salt)
  end

  private

  def hash(x)
    Digest::SHA2.hexdigest(x)
  end
end
