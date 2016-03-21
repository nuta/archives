Busybook
========
[![Build Status](https://travis-ci.org/seiyanuta/busybook.svg?branch=master)](https://travis-ci.org/seiyanuta/busybook)
[![Coverage Status](https://coveralls.io/repos/github/seiyanuta/busybook/badge.svg?branch=master)](https://coveralls.io/github/seiyanuta/busybook?branch=master)
[![Code Climate](https://codeclimate.com/github/seiyanuta/busybook/badges/gpa.svg)](https://codeclimate.com/github/seiyanuta/busybook)
[![Dependency Status](https://gemnasium.com/seiyanuta/busybook.svg)](https://gemnasium.com/seiyanuta/busybook)

Busybook is a CalDAV/CardDAV server out of the box powered by Ruby on Rails.

## Installation
```
$ apt-get > /dev/null || echo 'Use Ubuntu or Debian!'
$ sudo apt-get install wget git ruby2.0 ruby2.0-dev libsqlite3-dev zlib1g-dev

$ wget https://raw.githubusercontent.com/seiyanuta/busybook/master/init/busybook.init
$ sudo cp busybook.init /etc/init.d/busybook
$ sudo chmod 777 /etc/init.d/busybook
$ sudo service busybook setup
$ sudo service busybook start
```

## Configuration
- `/var/lib/busybook/database.yml`: Database connection configuration. By default it is a sqlite3 database. Refer [Ruby on Rails Guides](http://guides.rubyonrails.org/configuring.html#configuring-a-database).

## Backup
Busybook stores all data in the database so simply you can backup and restore by RDBMS's command like [pg_dump](http://www.postgresql.org/docs/9.5/static/app-pgdump.html).

## Compliance
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

## ToDo
- [ ] principals
- [ ] CardDAV
- [ ] documentation
- [ ] Web UI

## License
Public domain

## Changelog
- **v0.3.0**
  - remove cli tool
  - add a init script for Debian and Ubuntu
- **v0.2.1**
  - bug fixes
- **v0.2.0**
  - add busybook CLI
  - support Well-Known URI
- **v0.1.1**
  - fix implementation of calendar-query
- **v0.1.0**
  - support iOS
  - support calendar-query REPORT request
  - export a calendar as a .ics file
