require './lib/ics'

class Schedule < ActiveRecord::Base
  belongs_to :calendar

  validates :component,  presence: true
  validates :uri,        presence: true, uniqueness: true
  validates :ics,        presence: true

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

  def copy_to(calendar, calendar_object)
    ActiveRecord::Base.transaction do
      dst = Schedule.new(uri: calendar_object)
      dst.calendar = Calendar.find_by_uri!(calendar)
      dst.attributes = self.attributes.except('id', 'uri', 'calendar')

      dst.save!
    end
  end

  def move_to(calendar)
    self.calendar = Calendar.find_by_uri!(calendar)
    self.save!
  end

  def set_ics(body)
    ics = ICS::ICalendar.new(body)

    # accept a calendar event or a ToDo item
    unless %w(VEVENT VTODO).include?(ics.comp_type)
      # unknown calendar object
      raise "unsupported calendar object: '#{ics.comp_type}'"
    end

    self.ics        = body
    self.component  = ics.comp_type
    self.date_start = ics.comp('DTSTART', date: true)
    self.date_end   = ics.comp('DTEND',   date: true)
    self.summary    = ics.comp('SUMMARY')
  end
end
