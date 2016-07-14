class AddIndexUriToSchedule < ActiveRecord::Migration[4.2]
  def change
    add_index :schedules, :uri, unique: true
  end
end
