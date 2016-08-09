class AddIndexUriToCalendar < ActiveRecord::Migration[4.2]
  def change
    add_index :calendars, :uri, unique: true
  end
end
