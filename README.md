Busybook
========
[![Build Status](https://travis-ci.org/nuta/busybook.svg?branch=master)](https://travis-ci.org/nuta/busybook)
[![Code Climate](https://codeclimate.com/github/nuta/busybook/badges/gpa.svg)](https://codeclimate.com/github/nuta/busybook)
[![Dependency Status](https://gemnasium.com/nuta/busybook.svg)](https://gemnasium.com/nuta/busybook)

Busybook is a CalDAV/CardDAV server out of the box powered by Ruby on Rails.

## Features (and ToDo)
- [x] CalDAV
  - [x] HTTP Basic auth
  - [x] fundamental functions
  - [x] etag support (including `If-Match`)
  - [x] ctag support
- [x] rake command: add/delete/list users
- [x] rake command: export as a .ics/.vcard file
- [x] CalDAV/CardDAV tester
- [x] `.well-known` ([RFC5785](https://tools.ietf.org/html/rfc5785))
- [ ] Collection synchronization ([RFC6578](https://tools.ietf.org/html/rfc6578))
- [ ] principals
- [ ] CardDAV *WIP*
- [ ] easy deployment
  - [ ] `busybook(1)`
  - [ ] config files for `upstart`
  - [ ] config files for `systemd`
  - [ ] distribute as a gem
- [ ] documentation
- [ ] Web UI

## License
Public domain

## compliance
- [RFC4918: HTTP Extensions for Web Distributed Authoring and Versioning (WebDAV)](http://tools.ietf.org/html/rfc4918)
  - supports `GET`, `PUT`, `DELETE`, `OPTIONS`, `MKCALENDAR`, `PROPFIND`, and `PROPPATCH` HTTP methods
  - does *not* support `HEAD`, `POST`, `LOCK`, `UNLOCK`, `COPY`, `MOVE`, and `MKCOL` HTTP methods
- [RFC4791: Calendaring Extensions to WebDAV (CalDAV)](http://tools.ietf.org/html/rfc4791)
  - supports `REPORT` and `MKCALENDAR` HTTP method
- [RFC5545: iCalendar](http://tools.ietf.org/html/rfc5545)
  - supports `VEVENT` and `VTODO`
  - does *not* support `VJOURNAL`
- [caldav-ctag-03: Calendar Collection Entity Tag (CTag) in CalDAV](https://trac.calendarserver.org/browser/CalendarServer/trunk/doc/Extensions/caldav-ctag.txt)
  - supported
- [RFC5785: Defining Well-Known Uniform Resource Identifiers (URIs)](https://tools.ietf.org/html/rfc5785)
  - supported
- [RFC6638: Scheduling Extensions to CalDAV](http://tools.ietf.org/html/rfc6638)
  - *not* supported
- [RFC3744: WebDAV Access Control Protocol](https://tools.ietf.org/html/rfc3744)
  - *not* supported

## Changelog
- **v0.1.1**
  - fix implementation of calendar-query
- **v0.1.0**
  - support iOS
  - support calendar-query REPORT request
  - export a calendar as a .ics file
  - introduce [backup](https://github.com/backup/backup) gem
  - add unicorn config
