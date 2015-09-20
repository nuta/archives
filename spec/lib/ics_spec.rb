require 'spec_helper'
require 'ics'

describe ICS do
  describe 'concat_lines' do
    it 'concatenates iCalendar content lines (RFC5545, Section 3.1) correctly' do
      input = <<EOS
DESCRIPTION:This is 
 long long
  long line!
TZID:Asia/Tokyo\r
CREATED:20150919T093057Z
EOS
      output = [
        "DESCRIPTION:This is long long long line!",
        "TZID:Asia/Tokyo",
        "CREATED:20150919T093057Z"
      ]
      expect(ICS.send(:concat_lines, input)).to eq(output)
    end
  end

  it 'parses iCalendar correctly' do
    input = <<EOS
BEGIN:VCALENDAR
VERSION:2.0
PRODID:-//Apple Inc.//Mac OS X 10.10.5//EN
CALSCALE:GREGORIAN
BEGIN:VTIMEZONE
TZID:Asia/Tokyo
BEGIN:DAYLIGHT
TZOFFSETFROM:+0900
RRULE:FREQ=YEARLY;UNTIL=19510505T170000Z;BYMONTH=5;BYDAY=1SU
DTSTART:19500507T020000
TZNAME:GMT+9
TZOFFSETTO:+1000
END:DAYLIGHT
BEGIN:STANDARD
TZOFFSETFROM:+1000
DTSTART:19510908T020000
TZNAME:GMT+9
TZOFFSETTO:+0900
END:STANDARD
END:VTIMEZONE
BEGIN:VEVENT
CREATED:20150919T093057Z
UID:6016BB06-B428-47A6-80A5-A6F846D80AF1
DTEND;TZID=Asia/Tokyo:20150919T200000
TRANSP:OPAQUE
SUMMARY:Hello World
DTSTART;TZID=Asia/Tokyo:20150919T190000
DTSTAMP:20150919T093057Z
LOCATION:University of Tsukuba
SEQUENCE:0
END:VEVENT
END:VCALENDAR
EOS
    output = {
       "CALSCALE" => "GREGORIAN",
       "PRODID"   => "-//Apple Inc.//Mac OS X 10.10.5//EN",
       "VERSION"  => "2.0",
       "VEVENT"   => [{
         "CREATED"  => "20150919T093057Z",
         "UID"      => "6016BB06-B428-47A6-80A5-A6F846D80AF1",
         "TRANSP"   => "OPAQUE",
         "SUMMARY"  => "Hello World",
         "DTSTAMP"  => "20150919T093057Z",
         "LOCATION" => "University of Tsukuba",
         "SEQUENCE" => "0",
         "DTSTART;TZID=Asia/Tokyo" => "20150919T190000",
         "DTEND;TZID=Asia/Tokyo"   => "20150919T200000",
       }],
       "VTIMEZONE" => [{
         "TZID" => "Asia/Tokyo",
         "DAYLIGHT" => [{
           "RRULE"   => "FREQ=YEARLY;UNTIL=19510505T170000Z;" +
                        "BYMONTH=5;BYDAY=1SU",
           "DTSTART" => "19500507T020000",
           "TZNAME"  => "GMT+9",
           "TZOFFSETFROM" => "+0900",
           "TZOFFSETTO"   => "+1000"
         }],
         "STANDARD" => [{
           "DTSTART" => "19510908T020000",
           "TZNAME"  => "GMT+9",
           "TZOFFSETFROM" => "+1000",
           "TZOFFSETTO"   => "+0900"
         }]
       }]
    }

    expect(ICS::parse(input)).to eq(output)
  end
end
