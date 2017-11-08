class AddImageShasumToDeployment < ActiveRecord::Migration[5.1]
  def change
    add_column :deployments, :image_shasum, :string
  end
end
