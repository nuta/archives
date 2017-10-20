class AddOsVersionToApps < ActiveRecord::Migration[5.1]
  def change
    add_column :apps, :os_version, :string, null: false
  end
end
