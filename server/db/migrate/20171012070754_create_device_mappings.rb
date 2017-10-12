class CreateDeviceMappings < ActiveRecord::Migration[5.1]
  def change
    create_table :device_mappings do |t|
      t.references :device, foreign_key: true
      t.string :token, null: false
      t.string :type, null: false

      t.timestamps
    end
  end
end
