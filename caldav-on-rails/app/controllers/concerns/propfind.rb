module Propfind
  extend ActiveSupport::Concern

  def propfind_collections
    respond_xml_request('/A:propfind/A:prop/*') do |props|
      results = handle_props(props) do |prop|
        case prop
        when 'displayname'
          ''
        when 'calendar-home-set'
          '<A:href>/calendar/</A:href>'
        when 'principal-collection-set'
          '<A:href>/calendar/</A:href>'
        when 'principal-URL'
          '<A:href>/calendar/</A:href>'
        when 'getctag'
          s = @schedules.order('updated_at').last
          (s)? s.updated_at.to_i.to_s : ''
        when 'current-user-privilege-set'
          <<-EOS
            <A:privilege>
              <A:all />
            </A:privilege>
EOS
        when 'supported-report-set'
          <<-EOS
          <supported-report>
            <report><CALDAV:calendar-multiget /></report>
          </supported-report>
          <supported-report>
            <report><CALDAV:calendar-query /></report>
          </supported-report>
EOS
        when 'supported-calendar-component-set'
          <<-EOS
            <CALDAV:comp name="VTODO" />
            <CALDAV:comp name="VEVENT" />
EOS
        when 'resourcetype'
          <<-EOS
            <A:collection />
	    <A:principal />
EOS
        end
      end

      responses = [["/calendar/", results]]

      if params[:calendar] == ""
        if request.headers["Depth"] == "1"
          cals = @calendars.all
        else
          cals = []
        end
      else
        cals = [@calendars.find_by_uri!(params[:calendar])]
      end
     
      for cal in cals
        calprops = cal.props
        results = handle_props(props) do |prop|
          if calprops.key?(prop)
            calprops[prop]
          else
            case prop
            when 'getctag'
              s = @schedules.where(calendar: cal).order('updated_at').last
              (s)? s.updated_at.to_i.to_s : ''
            when 'current-user-privilege-set'
              <<-EOS
                <A:privilege>
                  <A:all />
                </A:privilege>
EOS
            when 'supported-report-set'
              <<-EOS
              <supported-report>
                <report><CALDAV:calendar-multiget /></report>
              </supported-report>
              <supported-report>
                <report><CALDAV:calendar-query /></report>
              </supported-report>
EOS
            when 'resourcetype'
              <<-EOS
                <CALDAV:calendar />
                <A:collection />
EOS
            end
          end
        end
        responses << ["/calendar/#{cal.uri}/", results]
      end

      responses
    end
  end

  def propfind_objects
    # get a list of calendar objects
    respond_xml_request('/A:propfind/A:prop/*') do |props|
      calendar = @calendars.find_by_uri!(params[:calendar])
      responses = []
      @schedules.where(calendar: calendar).find_each do |sched|
        results = handle_props(props) do |prop|
          case prop
          when 'getcontenttype'
            'text/calendar; component=vevent; charset=utf-8'
          when 'getetag'
            getetag(sched)
          end
        end
        responses << [sched.uri, results]
      end

      if responses == []
        results = handle_props(props) {|prop| }
        responses << ["/calendar/#{calendar.uri}/", results]
      end

      responses
    end
  end

  def handle_props(props)
    ps = {}
   for prop in props
     r = yield(prop.name)
     status = (r)? :ok : :not_found

     unless ps.has_key?(status)
       ps[status] = []
     end

     ps[status] << [prop.namespace.prefix, prop.name, r]
    end

    return ps
  end
end
