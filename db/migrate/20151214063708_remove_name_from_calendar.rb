class RemoveNameFromCalendar < ActiveRecord::Migration
  def change
    remove_column :calendars, :name, :string
  end
end
