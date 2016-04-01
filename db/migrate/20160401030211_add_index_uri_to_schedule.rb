class AddIndexUriToSchedule < ActiveRecord::Migration
  def change
    add_index :schedules, :uri, unique: true
  end
end
