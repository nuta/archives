HELP = """\
Busybook CLI

  create-guest - create a guest account for testing
  help         - show this message
"""

case ARGV[0]
when "create-guest"
  guest = User.new
  guest.name = "guest"
  guest.password = "password"
  guest.save
  cal = Calendar.new
  cal.uri = "123-456-789"
  cal.name = "Aiueo"
  cal.propxml = <<EOS
    {
        "calendar-color": "#7176FF",
        "displayname": "Aiueo"
    }
EOS
  cal.user = guest
  cal.save
else
  puts HELP
end
