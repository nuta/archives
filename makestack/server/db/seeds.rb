require 'ffaker'
require 'factory_bot_rails'

puts"Creating `test' user..."
FactoryBot.create(:user, username: "test", password: "12345678")

100.times do |i|
  puts "Creating an user (#{i + 1}/100)"
  u = FactoryBot.create(:heavy_user)
end
