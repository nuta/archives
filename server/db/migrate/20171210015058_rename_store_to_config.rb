class RenameStoreToConfig < ActiveRecord::Migration[5.1]
  def change
    rename_table :stores, :configs
  end
end
