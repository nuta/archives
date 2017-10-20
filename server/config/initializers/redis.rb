namespace = "mks:"
url = ENV['REDIS_URL'] || "redis://127.0.0.1:6379"
Redis.current = Redis::Namespace.new(namespace, redis: Redis.new(url: url))
