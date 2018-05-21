class RemoveUidFromSchedules < ActiveRecord::Migration[4.2]
  def change
    remove_column :schedules, :uid, :string
  end
end
