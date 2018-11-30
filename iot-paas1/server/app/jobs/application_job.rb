class ApplicationJob < ActiveJob::Base
  include Throttle

  def perform(*args)
    raise_if_reached_limit
    execute(*args)
  end
end
