class CreateCalendars < ActiveRecord::Migration[4.2]
  def change
    create_table :calendars do |t|
      t.string :name
      t.text :propxml
      t.references :user, index: true, foreign_key: true

      t.timestamps null: false
    end
  end
end
