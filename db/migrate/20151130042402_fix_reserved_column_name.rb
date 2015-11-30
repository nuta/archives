class FixReservedColumnName < ActiveRecord::Migration
  def change
    rename_column :schedules, :type, :component
  end
end
