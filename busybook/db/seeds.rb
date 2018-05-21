# This file should contain all the record creation needed to seed the database with its default values.
# The data can then be loaded with the rake db:seed (or created alongside the db with db:setup).
#
# Examples:
#
#   cities = City.create([{ name: 'Chicago' }, { name: 'Copenhagen' }])
#   Mayor.create(name: 'Emanuel', city: cities.first)

guest = User.create(name: 'guest', password: 'password')

Calendar.create(
  uri: '123-456-789',
  name: 'Aiueo',
  props_json: '{ "calendar-color": "#7176FF", "displayname": "Aiueo" }',
  user: guest
)
