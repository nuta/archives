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

  def append_log_to(target, device, lines, time)
    max_lines = APP_LOG_MAX_LINES
    if target == :app
      unless device.app
        # The device is not associated with any app. Aborting.
        return
      end

      log = app.log
      integrations = device.app.integrations.all
    else
      log = device.log
      integrations = []
    end

    device_name = self.name
    lines.each_with_index do |line, index|
      log["#{time}:#{index}:#{device_name}:#{line}"] = time
      m = /\A@(?<event>[^ ]+) (?<body>.*)\z/.match(line)
      if m
        # Detected a event published from the device.
        HookService.invoke(integrations, :event_published, self, {
          event: m['event'],
          body: m['body']
        })
      end
    end

    log.remrangebyrank(0, -max_lines)
  end

  def append_log(body)
    return unless body.is_a?(String)

    device_name = self.name
    time = Time.now.to_f
    lines = body.split("\n").reject(&:empty?)

    append_log_to(:device, self, lines, time)
    append_log_to(:app, self, lines, time)
  end
end
