class AddUriInCalendar < ActiveRecord::Migration[4.2]
  def change
    add_column :calendars, :uri, :string
  end
end
