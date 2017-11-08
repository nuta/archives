class AddDeployedFromToDeployments < ActiveRecord::Migration[5.1]
  def change
    add_column :deployments, :deployed_from, :string
  end
end
