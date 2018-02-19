class RenameStoreToConfig < ActiveRecord::Migration[5.1]
  def change
    safety_assured {
      rename_table :stores, :configs
    }
  end
end
