class Schedule < ActiveRecord::Base
  belongs_to :calendar

  validates :uri, uniqueness: true

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

  def Schedule.copy(src_path, calendar, calendar_object)
    ActiveRecord::Base.transaction do
      src = Schedule.find_by_uri!(src_path)
      dst = Schedule.new(uri: calendar_object)
      dst.calendar = Calendar.find_by_uri!(calendar)
      dst.attributes = src.attributes.except('id', 'uri', 'calendar')

      dst.save
      Change.create(calendar: dst.calendar, uri: dst.uri, is_delete: false)
    end
  end

  def Schedule.move(src_path, calendar, calendar_object)
    ActiveRecord::Base.transaction do
      src = Schedule.find_by_uri!(src_path)
      dst = Schedule.new(uri: calendar_object)
      dst.calendar = Calendar.find_by_uri!(calendar)
      dst.attributes = src.attributes.except('id', 'uri', 'calendar')

      src.destroy
      dst.save
      Change.create(calendar: src.calendar, uri: src.uri, is_delete: true)
      Change.create(calendar: dst.calendar, uri: dst.uri, is_delete: false)
    end
  end

  def save
    ActiveRecord::Base.transaction do
      super
      Change.create(calendar: self.calendar, uri: self.uri, is_delete: false)
    end
  end

  def destroy
    ActiveRecord::Base.transaction do
      Change.create(calendar: self.calendar, uri: self.uri, is_delete: true)
      super
    end
  end
end
