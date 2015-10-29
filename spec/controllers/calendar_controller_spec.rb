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
end
