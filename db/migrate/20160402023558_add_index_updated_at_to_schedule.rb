class AddIndexUpdatedAtToSchedule < ActiveRecord::Migration
  def change
    add_index :schedules, :updated_at
  end
end
