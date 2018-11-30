class DropChanges < ActiveRecord::Migration[4.2]
  def change
    drop_table :changes
  end
end
