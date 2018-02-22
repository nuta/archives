module Throttle
  extend ActiveSupport::Concern

  module ClassMethods
    attr_accessor :limit
    attr_accessor :period

    def throttle(limit:, period:)
      self.limit = limit
      self.period = period
    end
  end

  def raise_if_reached_limit
    redis_key = "job:rate:#{self.class.name.demodulize.downcase}"
    rate = Redis.current.get(redis_key).try(:to_i) || 0

    if rate >= self.class.limit
      raise "reached the job limit: key=#{redis_key}"
    end

    Redis.current.multi do
      Redis.current.incr(redis_key)
      if rate == 0
        Redis.current.expire(redis_key, self.class.period.to_i)
      end
    end
  end
end
