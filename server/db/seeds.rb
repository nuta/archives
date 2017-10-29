require 'ffaker'
require 'factory_girl_rails'

puts"Creating `test' user..."
FactoryGirl.create(:user, username: "test", password: "12345678")

100.times do |i|
  puts "Creating an user (#{i + 1}/100)"
  u = FactoryGirl.create(:heavy_user)
end
