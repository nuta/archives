class RenamePropxmlToPropjsonInCalendar < ActiveRecord::Migration[4.2]
  def change
    rename_column :calendars, :propxml, :props_json
  end
end
