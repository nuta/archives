Busybook
========
[![Build Status](https://travis-ci.org/seiyanuta/busybook.svg?branch=master)](https://travis-ci.org/seiyanuta/busybook)
[![Test Coverage](https://codeclimate.com/github/seiyanuta/busybook/badges/coverage.svg)](https://codeclimate.com/github/seiyanuta/busybook/coverage)
[![Code Climate](https://codeclimate.com/github/seiyanuta/busybook/badges/gpa.svg)](https://codeclimate.com/github/seiyanuta/busybook)

A CalDAV server built on Ruby on Rails.

## Securiy Notice
**Authorization is not implemented.** That is, an authenticated user is able to
get, update, and delete other users' calendars.

## Getting started
```
$ gem install bundler
$ bundle install --path vendor/bundle --jobs 4 --without postgres mysql
# cp config/database.yml.example config/database.yml
$ bundle exec rails db:migrate
$ bundle exec rails user:add username=chandler password=123
$ bundle exec rails s
```

![Add a CalDAV account on macOS](https://raw.githubusercontent.com/seiyanuta/busybook/master/add-account-on-macos.png)

## Deploying to Heroku
```
$ heroku git:remote -a <your_app_name>
$ git push heroku master
$ heroku run rails db:migrate
$ heroku run rails user:add username=<username password=<password>
```

## Supported clients
- OS X 10.12 (Sierra): Calendar and Reminder
- iOS 10: Calendar and Reminder

## Compliance
- [RFC4918: HTTP Extensions for Web Distributed Authoring and Versioning (WebDAV)](http://tools.ietf.org/html/rfc4918)
  - supports `GET`, `PUT`, `DELETE`, `OPTIONS`, `MKCALENDAR`, `PROPFIND`, `MOVE`, `COPY`, and `PROPPATCH` HTTP methods
  - does *not* support `HEAD`, `POST`, `LOCK`, `UNLOCK`, and `MKCOL` HTTP methods
- [RFC4791: Calendaring Extensions to WebDAV (CalDAV)](http://tools.ietf.org/html/rfc4791)
  - supports `REPORT` HTTP method and its `time-range` comp-filter
  - supports `MKCALENDAR` HTTP method
- [RFC5545: iCalendar](http://tools.ietf.org/html/rfc5545)
  - supports `VEVENT` and `VTODO`
  - does *not* support `VJOURNAL`
- [caldav-ctag-03: Calendar Collection Entity Tag (CTag) in CalDAV](https://trac.calendarserver.org/browser/CalendarServer/trunk/doc/Extensions/caldav-ctag.txt)
  - supported
- [RFC3744: WebDAV Access Control Protocol](https://tools.ietf.org/html/rfc3744)
  - supports some pseudo PROPFIND responses for Apple's implementations
- [RFC5785: Defining Well-Known Uniform Resource Identifiers (URIs)](https://tools.ietf.org/html/rfc5785)
  - supported
- [RFC6638: Scheduling Extensions to CalDAV](http://tools.ietf.org/html/rfc6638)
  - *not* supported

## Testing

```
$ bundle exec rails user:add username=guest password=123
$ rails s
```

```
$ bundle exec rails test:run
```

## License
Public domain

## Changelog
- **v1.1.2**
  - Upgrade Rails to 5.0.1
  - Support Ruby 2.4.0
- **v1.1.1**
  - Security fix: use [ActiveSupport::SecurityUtils#secure_compare](http://api.rubyonrails.org/classes/ActiveSupport/SecurityUtils.html#method-c-secure_compare) in password comparsion
- **v1.1.0**
  - Rails 5
- **v1.0.0**
  - the first stable version
- **v0.6.1**
  - support calendar subscriptions
  - fix MOVE method
- **v0.6.0**
  - fix /.well-known redirection
  - add Dockerfile
  - some improvements
- **v0.5.0**
  - init: update command
  - bug fixes
- **v0.4.0**
  - refactor
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
