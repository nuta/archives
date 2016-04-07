class RemoveUidFromSchedules < ActiveRecord::Migration
  def change
    remove_column :schedules, :uid, :string
  end
end
