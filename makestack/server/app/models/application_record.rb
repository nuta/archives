class ApplicationRecord < ActiveRecord::Base
  self.abstract_class = true

  include Redis::Objects

  after_destroy :delete_redis_objects

  def delete_redis_objects
    for key in self.redis_objects.keys
      Redis.current.del(self.send(key.to_s).key)
    end
  end
end
