module DeviceLog
  extend ActiveSupport::Concern

  APP_LOG_MAX_LINES = 512

  included do
    sorted_set :log, expiration: 1.hours
  end

  def get_log(since = 0)
    _lines = self.log.rangebyscore(since.to_f, Float::INFINITY) || []
    lines = _lines.join("\n").scrub("?").split("\n").map() do |l|
      c = l.split(":", 4)
      { time: c[0], index: c[1], device: c[2], body: c[3] }
    end

    lines
  end
end
