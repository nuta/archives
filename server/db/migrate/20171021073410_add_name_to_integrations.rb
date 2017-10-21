class AddNameToIntegrations < ActiveRecord::Migration[5.1]
  def change
    add_column :integrations, :name, :string, unique: true
  end
end
