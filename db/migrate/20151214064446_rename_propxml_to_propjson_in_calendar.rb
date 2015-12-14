class RenamePropxmlToPropjsonInCalendar < ActiveRecord::Migration
  def change
    rename_column :calendars, :propxml, :props_json
  end
end
