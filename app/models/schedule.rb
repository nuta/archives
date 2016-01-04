class Schedule < ActiveRecord::Base
  belongs_to :calendar

  def Schedule.in_time_range(calendar, range_start, range_end)
    sql = ''
    args = []

    if range_start
       sql += '? <= date_start '
       args << range_start
    end

    if range_end
	if sql != ""
	    sql += 'AND '
	end
	sql += 'date_end <= ? '
	args << range_end
    end

    self.where(calendar: Calendar.find_by_uri!(calendar)).where(sql, *args)
  end

  def save
    ActiveRecord::Base.transaction do
      super
      Change.create(calendar: self.calendar, uri: self.uri, is_delete: false)
    end
  end
end
