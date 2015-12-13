class User < ActiveRecord::Base
  validates :name, uniqueness: true
end
