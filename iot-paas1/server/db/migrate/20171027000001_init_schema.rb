class InitSchema < ActiveRecord::Migration[4.2]
  def change
    create_table "apps" do |t|
      t.string "name", null: false
      t.datetime "created_at", null: false
      t.datetime "updated_at", null: false
      t.bigint "user_id", null: false
      t.string "api", null: false
      t.string "os_version", null: false
      t.index ["user_id"], name: "index_apps_on_user_id"
    end

    create_table "deployments" do |t|
      t.integer "version", null: false
      t.string "tag"
      t.string "comment"
      t.string "debug"
      t.string "deployed_from"
      t.string "image_shasum"
      t.bigint "app_id", null: false
      t.datetime "created_at", null: false
      t.datetime "updated_at", null: false
      t.binary "image", null: false
      t.index ["app_id"], name: "index_deployments_on_app_id"
      t.index ["app_id", "version"], name: "index_deployments_on_app_id_and_version", unique: true
    end

    create_table "devices" do |t|
      t.string "name", null: false
      t.string "device_id", null: false
      t.string "device_id_prefix", null: false
      t.string "device_secret", null: false
      t.string "tag"
      t.string "device_type", null: false
      t.string "sakuraio_module_token"
      t.bigint "user_id", null: false
      t.bigint "app_id"
      t.datetime "created_at", null: false
      t.datetime "updated_at", null: false
      t.index ["app_id"], name: "index_devices_on_app_id"
      t.index ["device_id_prefix"], name: "index_devices_on_device_id_prefix", unique: true
      t.index ["user_id"], name: "index_devices_on_user_id"
      t.index ["user_id", "name"], name: "index_devices_on_user_id_and_name", unique: true
    end

    create_table "device_mappings" do |t|
      t.bigint "device_id", null: false
      t.string :token, null: false
      t.string :token_type, null: false
      t.datetime "created_at", null: false
      t.datetime "updated_at", null: false
      t.index ["device_id"], name: "index_device_mappings_on_device_id"
      t.index ["token"], name: "index_device_mappings_on_token"
    end

    create_table "integrations" do |t|
      t.string "service", null: false
      t.text "config", null: false
      t.datetime "created_at", null: false
      t.datetime "updated_at", null: false
      t.bigint "app_id", null: false
      t.string "name", null: false
      t.string "comment"
      t.string "token"
      t.string "token_prefix"
      t.index ["app_id"], name: "index_integrations_on_app_id"
      t.index ["token_prefix"], name: "index_integrations_on_token_prefix"
    end

    create_table "source_files" do |t|
      t.string "path", null: false
      t.text "body"
      t.bigint "app_id", null: false
      t.datetime "created_at", null: false
      t.datetime "updated_at", null: false
      t.index ["app_id"], name: "index_source_files_on_app_id"
      t.index ["app_id", "path"], name: "index_source_files_on_app_id_and_path"
    end

    create_table "stores" do |t|
      t.string "key", null: false
      t.string "value"
      t.bigint "owner_id"
      t.string "owner_type"
      t.datetime "created_at", null: false
      t.datetime "updated_at", null: false
      t.string "data_type", null: false
      t.index ["owner_type", "owner_id",], name: "index_stores_on_owner_type_and_owner_id"
      t.index ["owner_type", "owner_id", "key"], name: "index_stores_on_owner_type_and_owner_id_and_key", unique: true
    end

    create_table "users" do |t|
      t.string "provider", default: "email", null: false
      t.string "uid", default: "", null: false
      t.string "encrypted_password", default: "", null: false
      t.string "reset_password_token"
      t.datetime "reset_password_sent_at"
      t.datetime "remember_created_at"
      t.integer "sign_in_count", default: 0, null: false
      t.datetime "current_sign_in_at"
      t.datetime "last_sign_in_at"
      t.string "current_sign_in_ip"
      t.string "last_sign_in_ip"
      t.string "confirmation_token"
      t.datetime "confirmed_at"
      t.datetime "confirmation_sent_at"
      t.string "unconfirmed_email"
      t.string "name"
      t.string "nickname"
      t.string "image"
      t.string "email", null: false
      t.text "tokens"
      t.datetime "created_at", null: false
      t.datetime "updated_at", null: false
      t.string "username", null: false
      t.index ["confirmation_token"], name: "index_users_on_confirmation_token", unique: true
      t.index ["email"], name: "index_users_on_email", unique: true
      t.index ["reset_password_token"], name: "index_users_on_reset_password_token", unique: true
      t.index ["uid", "provider"], name: "index_users_on_uid_and_provider", unique: true
      t.index ["username"], name: "index_users_on_username", unique: true
    end

    add_foreign_key "apps", "users"
    add_foreign_key "deployments", "apps"
    add_foreign_key "devices", "apps"
    add_foreign_key "devices", "users"
    add_foreign_key "device_mappings", "devices"
    add_foreign_key "integrations", "apps"
    add_foreign_key "source_files", "apps"
  end
end
