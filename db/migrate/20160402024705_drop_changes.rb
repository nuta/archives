class DropChanges < ActiveRecord::Migration
  def change
    drop_table :changes
  end
end
