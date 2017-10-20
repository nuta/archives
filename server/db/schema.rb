# This file is auto-generated from the current state of the database. Instead
# of editing this file, please use the migrations feature of Active Record to
# incrementally modify your database, and then regenerate this schema definition.
#
# Note that this schema.rb definition is the authoritative source for your
# database schema. If you need to create the application database on another
# system, you should be using db:schema:load, not running all the migrations
# from scratch. The latter is a flawed and unsustainable approach (the more migrations
# you'll amass, the slower it'll run and the greater likelihood for issues).
#
# It's strongly recommended that you check this file into your version control system.

ActiveRecord::Schema.define(version: 20171020055609) do

  create_table "app_stores", force: :cascade do |t|
    t.string "key", null: false
    t.string "value"
    t.bigint "app_id", null: false
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
    t.string "data_type", null: false
    t.index ["app_id", "key"], name: "index_app_stores_on_app_id_and_key", unique: true
    t.index ["app_id"], name: "index_app_stores_on_app_id"
  end

  create_table "apps", force: :cascade do |t|
    t.string "name", null: false
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
    t.bigint "user_id", null: false
    t.string "api", null: false
    t.string "os_version"
    t.index ["user_id"], name: "index_apps_on_user_id"
  end

  create_table "deployments", force: :cascade do |t|
    t.integer "version", null: false
    t.string "tag"
    t.string "comment"
    t.string "debug"
    t.bigint "app_id", null: false
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
    t.binary "image", null: false
    t.index ["app_id", "version"], name: "index_deployments_on_app_id_and_version", unique: true
    t.index ["app_id"], name: "index_deployments_on_app_id"
  end

  create_table "device_mappings", force: :cascade do |t|
    t.integer "device_id"
    t.string "token", null: false
    t.string "type", null: false
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
  end

  create_table "device_stores", force: :cascade do |t|
    t.string "key", null: false
    t.string "value"
    t.bigint "device_id", null: false
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
    t.string "data_type", null: false
    t.index ["device_id", "key"], name: "index_device_stores_on_device_id_and_key", unique: true
    t.index ["device_id"], name: "index_device_stores_on_device_id"
  end

  create_table "devices", force: :cascade do |t|
    t.string "name", null: false
    t.string "device_id", null: false
    t.string "device_id_prefix", null: false
    t.string "device_secret", null: false
    t.string "tag"
    t.string "device_type", null: false
    t.bigint "user_id", null: false
    t.bigint "app_id"
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
    t.index ["app_id"], name: "index_devices_on_app_id"
    t.index ["device_id_prefix"], name: "index_devices_on_device_id_prefix", unique: true
    t.index ["user_id", "name"], name: "index_devices_on_user_id_and_name", unique: true
    t.index ["user_id"], name: "index_devices_on_user_id"
  end

  create_table "integrations", force: :cascade do |t|
    t.string "service", null: false
    t.text "config", null: false
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
    t.bigint "app_id", null: false
    t.string "comment"
    t.string "token"
    t.string "token_prefix"
    t.index ["app_id"], name: "index_integrations_on_app_id"
  end

  create_table "source_files", force: :cascade do |t|
    t.string "path", null: false
    t.text "body"
    t.bigint "app_id", null: false
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
    t.index ["app_id", "path"], name: "index_source_files_on_app_id_and_path"
    t.index ["app_id"], name: "index_source_files_on_app_id"
  end

  create_table "users", force: :cascade do |t|
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

end
