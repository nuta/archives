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
<?xml version="1.0" encoding="UTF-8"?>
<B:mkcalendar xmlns:B="urn:ietf:params:xml:ns:caldav">
  <A:set xmlns:A="DAV:">
    <A:prop>
      <D:calendar-color xmlns:D="http://apple.com/ns/ical/" symbolic-color="purple">
      #7176FF
      </D:calendar-color>
      <A:displayname>Aiueo</A:displayname>
    </A:prop>
  </A:set>
</B:mkcalendar>
EOS
  cal.user = guest
  cal.save
else
  puts HELP
end
