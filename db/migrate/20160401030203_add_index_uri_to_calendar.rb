class AddIndexUriToCalendar < ActiveRecord::Migration
  def change
    add_index :calendars, :uri, unique: true
  end
end
