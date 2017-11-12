class AddSakuraioModuleTokenToDevices < ActiveRecord::Migration[5.1]
  def change
    add_column :devices, :sakuraio_module_token, :string
  end
end
