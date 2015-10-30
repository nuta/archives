# coding: utf-8
require 'rails_helper'


def send_request(method, body, **params)

  request.env['RAW_POST_DATA'] = body
  process method.downcase.to_sym, method, **params
end


RSpec.describe CalendarController, type: :controller do
  let(:user) { create(:user) }

  describe 'OPTIONS' do
    it "responds successfully" do
      send_request('OPTIONS', '', {:user => '', :uri => '/'})
      expect(response).to have_http_status(200)
      expect(response.header).to include('DAV')
   end
  end

  describe 'PROPFIND /' do
    let(:body) { <<EOS
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
    }


    it "responds successfully" do
      send_request('PROPFIND', body, {:user => user.name, :uri => '/'})
      expect(response).to have_http_status(207)
      expect(response.body).to include("<status>HTTP/1.1 200 OK</status>")
    end
  end

  describe 'MKCALENDAR' do
    let(:body) { <<EOS
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
    }

    it "creates a calendar" do
      send_request('MKCALENDAR', body, {:user => user.name, :uri => 'blah'})
      expect(response).to have_http_status(:created)
  
      calendar = Calendar.where(name: 'My Work', user: User.find_by_name(user.name))
      expect(calendar).to exist
    end
  end


  describe 'PUT /:user/calendars/:uri' do
    let(:body) { <<EOS
  BEGIN:VCALENDAR
  END:VCALENDAR
EOS
    }

    it "creates a object" do
      send_request('PUT', body, {:user => user.name, :uri => 'foo.ics'})
      expect(response).to have_http_status(:created)

      schedule = Schedule.where(uri: 'foo.ics').first
      expect(schedule.ics).to eq(body)
    end
  end


  describe 'DELETE /:user/calendars/:uri' do
    before { @object = create(:schedule) }

    it "deletes a object" do
      process :delete, 'DELETE', :user => user.name, :uri => @object.uri
      expect(response).to have_http_status(:no_content)
    end
  end
end
