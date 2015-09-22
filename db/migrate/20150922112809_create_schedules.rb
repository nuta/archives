class CreateSchedules < ActiveRecord::Migration
  def change
    create_table :schedules do |t|
      t.string :type
      t.text :summary
      t.datetime :date_start
      t.datetime :date_end
      t.string :uri
      t.string :uid
      t.text :ics
      t.references :calendar, index: true, foreign_key: true

      t.timestamps null: false
    end
  end
end
