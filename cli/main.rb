HELP = """\
Busybook CLI

  create-guest - create a guest account for testing
  help         - show this message
"""

case ARGV[0]
when "create-guest"
  guest = User.new
  guest.name = "guest"
  guest.save
else
  puts HELP
end
