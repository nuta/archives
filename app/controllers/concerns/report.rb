module Report
  extend ActiveSupport::Concern

  def report_query(xml)
    respond_xml_request('/C:calendar-query/A:prop/*') do |props|
      filters = xml.xpath('/C:calendar-query/C:filter/C:comp-filter/C:comp-filter/*',
                        C: 'urn:ietf:params:xml:ns:caldav')
      scheds = []
      filters.each do |filter|
        if filter.name == 'time-range'
          # TODO: support other comp-filters
          range_start = Time.zone.parse(filter.attr('start') || '')
          range_end   = Time.zone.parse(filter.attr('end')   || '')
          scheds = Schedule.in_time_range(params[:calendar], range_start, range_end)
          break
        end
      end

      responses = []
      for sched in scheds
        calendar_uri ||= sched.calendar.uri
        results = handle_props(props) do |prop|
          case prop
          when 'getetag'
            getetag(sched)
          when 'calendar-data'
	    sched.ics.encode(xml: :text)
          end
        end
        responses << ["/calendar/#{calendar_uri}/#{sched.uri}", results]
      end
      responses
    end
  end

  def report_multiget(xml)
    respond_xml_request('/C:calendar-multiget/A:prop/*') do |props|
      uris  = xml.xpath('/C:calendar-multiget/A:href',
                        A: 'DAV:', C: 'urn:ietf:params:xml:ns:caldav')

      responses = []
      for uri in uris
        # TODO: return 404 reponse if it does not exists
        sched = Schedule.find_by_uri!(URI.decode(File.basename(uri)))

        results = handle_props(props) do |prop|
          case prop
          when 'getetag'
            getetag(sched)
          when 'calendar-data'
	    sched.ics.encode(xml: :text)
          end
        end
        responses << [uri.text, results]
      end
      responses
    end
  end
end
