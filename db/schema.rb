# encoding: UTF-8
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

ActiveRecord::Schema.define(version: 20150922112809) do

  create_table "calendars", force: :cascade do |t|
    t.string   "name"
    t.text     "propxml"
    t.integer  "user_id"
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
  end

  add_index "calendars", ["user_id"], name: "index_calendars_on_user_id"

  create_table "schedules", force: :cascade do |t|
    t.string   "type"
    t.text     "summary"
    t.datetime "date_start"
    t.datetime "date_end"
    t.string   "uri"
    t.string   "uid"
    t.text     "ics"
    t.integer  "calendar_id"
    t.datetime "created_at",  null: false
    t.datetime "updated_at",  null: false
  end

  add_index "schedules", ["calendar_id"], name: "index_schedules_on_calendar_id"

  create_table "users", force: :cascade do |t|
    t.string   "name"
    t.datetime "created_at", null: false
    t.datetime "updated_at", null: false
  end

end
