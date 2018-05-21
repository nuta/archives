class RemoveNameFromCalendar < ActiveRecord::Migration[4.2]
  def change
    remove_column :calendars, :name, :string
  end
end
