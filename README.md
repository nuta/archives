Busybook
========
[![Build Status](https://travis-ci.org/nuta/busybook.svg?branch=master)](https://travis-ci.org/nuta/busybook)
[![Coverage Status](https://coveralls.io/repos/nuta/busybook/badge.svg?branch=master&service=github)](https://coveralls.io/github/nuta/busybook?branch=master)
[![Code Climate](https://codeclimate.com/github/nuta/busybook/badges/gpa.svg)](https://codeclimate.com/github/nuta/busybook)
[![Dependency Status](https://gemnasium.com/nuta/busybook.svg)](https://gemnasium.com/nuta/busybook)

Busybook is a CalDAV/CardDAV server out of the box powered by Ruby on Rails.

## Features (ToDo)
- [ ] CalDAV
  - [x] fundamental functions
  - [ ] ctag support
  - [ ] test by [CalDAVTester](http://calendarserver.org/wiki/CalDAVTester)
- [ ] CardDAV
- [ ] command-line utility (add user, backup)
- [ ] write documentation

## License
[MIT License](http://opensource.org/licenses/mit-license.php)

## compliance
- [RFC4918: HTTP Extensions for Web Distributed Authoring and Versioning (WebDAV)](http://tools.ietf.org/html/rfc4918)
  - supports `GET`, `PUT`, `DELETE`, `OPTIONS`, `PROPFIND`, and `PROPPATCH` HTTP methods
  - does not support `HEAD`, `POST`, `LOCK`, `UNLOCK`, `COPY`, `MOVE`, and `MKCOL` HTTP methods
- [RFC3744: WebDAV Access Control Protocol](https://tools.ietf.org/html/rfc3744)
  - not supported
- [RFC4791: Calendaring Extensions to WebDAV (CalDAV)](http://tools.ietf.org/html/rfc4791)
  - supports `REPORT` and `MKCALENDAR` HTTP method
- [RFC5545: iCalendar](http://tools.ietf.org/html/rfc5545)
  - supports `VEVENT` and `VTODO`
  - does not support `VJOURNAL`
- [RFC6638: Scheduling Extensions to CalDAV](http://tools.ietf.org/html/rfc6638)
  - not supported
- [caldav-ctag-03: Calendar Collection Entity Tag (CTag) in CalDAV](https://trac.calendarserver.org/browser/CalendarServer/trunk/doc/Extensions/caldav-ctag.txt)
  - not supported
