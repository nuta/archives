namespace = "mks:"
url = ENV['REDIS_URL'] || "redis://127.0.0.1:6379"

class Redis
  def self.__current
    @__current
  end

  def self.__current=(__current)
    @__current = __current
  end
end

Redis.__current = Redis.new(url: url)
Redis.current = Redis::Namespace.new(namespace, redis: Redis.__current)
