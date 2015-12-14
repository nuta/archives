class Schedule < ActiveRecord::Base
  belongs_to :calendar

  def Schedule.time_range(range_start, range_end)
    range_start ||= Time.now - 100000.years
    range_end   ||= Time.now + 100000.years

    self.where('? <= date_start AND date_end <= ?', range_start, range_end)
  end
end
