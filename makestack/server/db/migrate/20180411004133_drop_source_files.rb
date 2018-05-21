class DropSourceFiles < ActiveRecord::Migration[5.1]
  def change
    drop_table :source_files
  end
end
