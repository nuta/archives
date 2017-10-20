require 'factory_girl'

puts"Creating `test' user..."
FactoryGirl.create(:user, username: "test", password: "12345678")

100.times do |i|
  puts "Creating an user (#{i + 1}/100)"
  u = FactoryGirl.create(:user)
  10.times { FactoryGirl.create(:app, user: u) }
  10.times { FactoryGirl.create(:device, user: u) }
end
