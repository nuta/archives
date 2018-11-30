class AddIndexUpdatedAtToSchedule < ActiveRecord::Migration[4.2]
  def change
    add_index :schedules, :updated_at
  end
end
