class AddUriInCalendar < ActiveRecord::Migration
  def change
    add_column :calendars, :uri, :string
  end
end
