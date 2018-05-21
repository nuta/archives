class FixReservedColumnName < ActiveRecord::Migration[4.2]
  def change
    rename_column :schedules, :type, :component
  end
end
