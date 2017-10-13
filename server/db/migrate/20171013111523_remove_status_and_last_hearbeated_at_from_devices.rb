class RemoveStatusAndLastHearbeatedAtFromDevices < ActiveRecord::Migration[5.1]
  def change
    remove_column :devices, :status
    remove_column :devices, :last_heartbeated_at
  end
end
