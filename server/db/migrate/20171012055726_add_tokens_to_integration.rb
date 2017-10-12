class AddTokensToIntegration < ActiveRecord::Migration[5.1]
  def change
    add_column :integrations, :token, :string
    add_column :integrations, :token_prefix, :string
  end
end
