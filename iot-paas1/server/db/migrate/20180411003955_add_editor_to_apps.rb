class AddEditorToApps < ActiveRecord::Migration[5.1]
  def change
    # We don't have large-scale production environments yet.
    safety_assured do
      add_column :apps, :editor, :string, default: 'code'
    end
  end
end
