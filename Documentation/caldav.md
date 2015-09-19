CalDAV
======
This document describes a CalDAV, a protocol based on WebDAV to
acccess schedules.


Related RFCs
------------
- [RFC4791](https://tools.ietf.org/html/rfc4791): Calendaring Extensions to WebDAV (CalDAV)
- [RFC2518](https://tools.ietf.org/html/rfc2518): HTTP Extensions for Distributed Authoring -- WEBDAV
- [RFC3253](https://tools.ietf.org/html/rfc3253): Versioning Extensions to WebDAV
- [RFC3744](https://tools.ietf.org/html/rfc3744): WebDAV Access Control Protocol
- [RFC6638](https://tools.ietf.org/html/rfc6638): Scheduling Extensions to CalDAV

HTTP Methods
------------

### OPTIONS
In OPTIONS method, A CalDAV server MUST include `calendar-access` in
`DAV` response header to indicate that it supports CalDAV.

Example (request and response):
```
OPTIONS / HTTP/1.1
Host: cal.example.com


HTTP/1.1 200 OK
Date: Sat, 19 Sep 2015 09:06:23 GMT
DAV: 1, 2, 3, calendar-access, addressbook, extended-mkcol
Allow: DELETE, HEAD, GET, MKCALENDAR, MKCOL, MOVE, OPTIONS
Allow: PROPFIND, PROPPATCH, PUT, REPORT
Content-Length: 0
```

### MKCALENDAR
It creates a new calendar. A request body (optional) MUST be a
`CALDAV:mkcalendar` XML element.

Example (request from OS X Calendar):
```
MKCALENDAR /calendar/7287558F-5F1C-4AEC-9F3C-E9C1068D4E4E/ HTTP/1.1
Host: cal.example.com
UserAgent: Mac+OS+X/10.10.5 (14F27) CalendarAgent/316.1

<?xml version="1.0" encoding="UTF-8"?>
<B:mkcalendar xmlns:B="urn:ietf:params:xml:ns:caldav">
  <A:set xmlns:A="DAV:">
    <A:prop>
    
      <D:calendar-order xmlns:D="http://apple.com/ns/ical/">
      10
      </D:calendar-order>
      
      <B:supported-calendar-component-set>
        <B:comp name="VEVENT"/>
      </B:supported-calendar-component-set>
      
      <D:calendar-color xmlns:D="http://apple.com/ns/ical/" symbolic-color="purple">
      #711A76FF
      </D:calendar-color>
      
      <B:calendar-timezone>
        VTIMEZONE is here
      </B:calendar-timezone>
      
      <A:displayname>Untitled 2</A:displayname>
      
      <B:calendar-free-busy-set>
        <YES/>
      </B:calendar-free-busy-set>
      
    </A:prop>
  </A:set>
</B:mkcalendar>
```

Example (response):
```
HTTP/1.1 201 Created
Cache-Control:no-cache
Content-Length: 0
```

### PUT
It creates/updates a calendar object such as an event and a ToDo. The
server just create/overwrite a file at the specified path like WebDAV.

This method is defined in WebDAV RFC.

Not to overwrite existing resource inadvertently, client can use
`If-None-Match`.

Example (request):
```txt
PUT /calendar/6016BB06-B428-47A6-80A5-A6F846D80AF1.ics HTTP/1.1
If-None-Match: *
Host: cal.example.com
Content-Type: text/calendar
         
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
```

Example (response):
```
HTTP/1.1 201 Created
Content-Length: 0
```

### DELETE
It deletes a calendar object. This method is defined in WebDAV RFC.

Example:
```
DELETE /calendar/6016BB06-B428-47A6-80A5-A6F846D80AF1.ics HTTP/1.1
Host: cal.example.com


HTTP/1.1 200 OK
```


### PROPFIND
It finds objects. This method is defined in WebDAV RFC.


### REPORT
It also finds objects. This method is defined in an extension of
WebDAV RFC (3253).  Since OS X Calendar does not use it, I ignore it
for now.
