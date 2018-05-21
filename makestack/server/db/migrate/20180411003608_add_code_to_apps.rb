class AddCodeToApps < ActiveRecord::Migration[5.1]
  def change
    add_column :apps, :code, :string
  end
end
