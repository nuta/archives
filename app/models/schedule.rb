class Schedule < ActiveRecord::Base
  belongs_to :calendar

  def time_range(range_start, range_end)
    range_start ||= Time.now - 100000.years
    range_end   ||= Time.now + 100000.years

    self.where("? <= start AND end <= ?", range_start, range_end)
  end
end
