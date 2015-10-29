# coding: utf-8
require 'rails_helper'

RSpec.describe CalendarController, type: :controller do
  let!(:user) { create(:user) }

  it 'responds OPTIONS requests' do
    process :options, 'OPTIONS', :user => user.name, :uri => '/'
    expect(response).to have_http_status(200)
    expect(response.header).to include('DAV')
  end


  it 'responds a PROPFIND request (get a list of calendars)' do
    headers = {
      'Content-Type' => 'text/xml',
      'Depth'        => '0',
      'Brief'        => 't',
      'Accept'       => '*/*',
      'Prefer'       => 'return=minimal'
    }

    body = <<EOS
<?xml version="1.0" encoding="UTF-8"?>
<A:propfind xmlns:A="DAV:">
  <A:prop>
    <B:calendar-home-set xmlns:B="urn:ietf:params:xml:ns:caldav"/>
    <B:calendar-user-address-set xmlns:B="urn:ietf:params:xml:ns:caldav"/>
    <A:current-user-principal/>
    <A:displayname/>
    <C:dropbox-home-URL xmlns:C="http://calendarserver.org/ns/"/>
    <C:email-address-set xmlns:C="http://calendarserver.org/ns/"/>
    <C:notification-URL xmlns:C="http://calendarserver.org/ns/"/>
    <A:principal-collection-set/>
    <A:principal-URL/>
    <A:resource-id/>
    <B:schedule-inbox-URL xmlns:B="urn:ietf:params:xml:ns:caldav"/>
    <B:schedule-outbox-URL xmlns:B="urn:ietf:params:xml:ns:caldav"/>
    <A:supported-report-set/>
  </A:prop>
</A:propfind>
EOS

    request.env['RAW_POST_DATA'] = body
    process :propfind, 'PROPFIND', :user => user.name, :uri => '/'
    expect(response).to have_http_status(207)
    expect(response.body).to include("<status>HTTP/1.1 200 OK</status>")
  end


  it 'creates a calendar' do
    headers = {
      'Content-Type' => 'text/xml',
      'Depth'        => '0',
      'Brief'        => 't',
      'Accept'       => '*/*',
      'Prefer'       => 'return=minimal'
    }

    body = <<EOS
<?xml version="1.0" encoding="UTF-8"?>
<B:mkcalendar xmlns:B="urn:ietf:params:xml:ns:caldav">
  <A:set xmlns:A="DAV:">
    <A:prop>
      <D:calendar-color xmlns:D="http://apple.com/ns/ical/" symbolic-color="purple">
      #711A76FF
      </D:calendar-color>
      <A:displayname>My Work</A:displayname>
    </A:prop>
  </A:set>
</B:mkcalendar>
EOS

    request.env['RAW_POST_DATA'] = body
    process :mkcalendar, 'MKCALENDAR', :user => user.name,
            :uri => '/calendar/7287558F-5F1C-4AEC-9F3C-E9C1068D4E4E/'
    expect(response).to have_http_status(201)

    expect(Calendar.where(name: 'My Work', user: User.find_by_name(user.name))).to exist
  end
end
