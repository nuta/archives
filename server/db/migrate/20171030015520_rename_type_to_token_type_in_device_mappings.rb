class RenameTypeToTokenTypeInDeviceMappings < ActiveRecord::Migration[5.1]
  def change
    rename_column :device_mappings, :type, :token_type
  end
end
