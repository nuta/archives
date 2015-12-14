require 'digest/md5'

class CalendarController < ApplicationController
  before_action :authenticate, except: [:options]

  def options
    methods = %w(OPTIONS GET PUT DELETE MKCALENDAR PROPFIND PROPPATCH REPORT)
    headers['Allow'] = methods.join(', ')
    headers['DAV']   = '1, 2, calendar-access'
    head :ok
  end

  def get
    entry = Schedule.find_by_uri!(params[:calendar_object])
    render :text => entry.ics, :status => :ok
  end

  def put
    body = request_body
    ics  = ICS::parse(body)
    uri  = params[:calendar_object]

    # determine component type
    comp_name = ''
    for comp_type in %w(VEVENT VTODO)
      if ics.has_key?(comp_type)
        comp_name = comp_type
        break
      end
    end

    unless comp_name
      # unknown calendar object
      head :status => :not_implemented
      return
    end

    comp = ics[comp_name][0]
    date_start = ICS::parse_date(comp, 'DTSTART')
    date_end   = ICS::parse_date(comp, 'DTEND')
    calendar   = Calendar.find_by_uri!(params[:calendar])

    sched = Schedule.where(uri: uri).first

    if request.headers.key?("If-None-Match") and sched
      head :status => :precondition_failed
      return
    end

    if request.headers.key?("If-Match") and
       sched and getetag(sched) != remove_etag_prefix(request.headers["If-Match"])
      head :status => :precondition_failed
      return
    end

    unless sched
      sched = Schedule.new
      sched.uri = uri
    end

    sched.ics        = body
    sched.component  = comp_name
    sched.date_start = date_start
    sched.date_end   = date_end
    sched.calendar   = calendar
    sched.uid        = comp['uid']
    sched.summary    = comp['summary']

    ActiveRecord::Base.transaction do
      sched.save
      Change.create(calendar: calendar, uri: uri, is_delete: false)
    end

    head :status => :created
  end

  def delete
    cal = Calendar.find_by_uri!(params[:calendar])
    if params[:calendar_object] == ""
      cal.delete
    else
      ActiveRecord::Base.transaction do
        sched = Schedule.find_by_uri!(params[:calendar_object])
        sched.delete
        Change.create(calendar: cal, uri: sched.uri, is_delete: true)
      end
    end

    head :status => :no_content
  end

  def mkcalendar
    xml = Nokogiri::XML(request_body)

    calendar = Calendar.new
    props = {}

    for prop in xml.xpath('/B:mkcalendar/A:set/A:prop/*',
                          A: 'DAV:', B: 'urn:ietf:params:xml:ns:caldav')

      props[prop.name] = replace_propxml_nsprefix(xml, prop.children.to_s)

      if prop.name == 'displayname'
        name = props[prop.name]
      end
    end

    calendar.props   = props
    calendar.uri     = params[:calendar]
    calendar.name    = name
    calendar.user    = @user

    ActiveRecord::Base.transaction do
      calendar.save
      Change.create(calendar: calendar, uri: '', is_delete: false)
    end

    head :status => :created
  end

  def report
    xml = Nokogiri::XML(request_body)
    res = case xml.children[0].name
          when 'calendar-multiget'
            report_multiget(xml)
          when 'calendar-query'
            report_query(xml)
          else
            head :status => :not_implemented
            return
          end

    render :xml => res, :status => :multi_status
  end

  def proppatch
    if params[:calendar_object] != ""
      # PROPPATCH to a calendar object is not supported yet
      head :status => :not_implemented
      return
    end

    if params[:calendar] != ""
      xml = Nokogiri::XML(request_body)
      cal = Calendar.find_by_uri!(params[:calendar])
      calprops = cal.props
  
      for prop in xml.xpath('/A:propertyupdate/A:set/A:prop/*', A: 'DAV:')
        calprops[prop.name] = replace_propxml_nsprefix(xml, prop.children.to_s)
      end
  
      for prop in xml.xpath('/A:propertyupdate/A:remove/A:prop/*', A: 'DAV:')
        calprops.delete(prop.name)
      end

      cal.props = calprops
    
      ActiveRecord::Base.transaction do
        cal.save
        Change.create(calendar: cal, uri: '', is_delete: false)
      end
    end

    res = respond_xml_request('/A:propertyupdate/*/A:prop/*') do |props|
      results = handle_props(props) {|prop| '' }
      [["/calendar/#{params[:calendar]}/", results]]
    end

    render :xml => res, :status => :multi_status
  end

  def propfind
    if params[:calendar] != '' and request.headers["Depth"] == "1"
      xml = propfind_objects
    else
      xml = propfind_collections
    end

    render :xml => xml, :status => :multi_status
  end

  private

  def getetag(sched)
    '"' + Digest::MD5.hexdigest(sched.ics) + '"'
  end

  def remove_etag_prefix(etag)
    if etag.start_with?("W/")
      etag[2..-1]
    else
      etag
    end
  end

  def report_query(xml)
    respond_xml_request('/C:calendar-query/A:prop/*') do |props|
      uris  = xml.xpath('/C:calendar-query/C:filter/C:comp-filter/*',
                        C: 'urn:ietf:params:xml:ns:caldav')

      scheds = []
      filters.each do |filter|
        if filter.name == 'time-range'
          # TODO: support other comp-filters
          range_start = Time.zone.parse(filter.attr('start'))
          range_end   = Time.zone.parse(filter.attr('end'))
          scheds = Schedule.time_range(range_start, range_end)
          break
        end
      end

      responses = []
      for sched in schds
        results = handle_props(props) do |prop|
          case prop
          when 'getetag'
            getetag(sched)
          when 'calendar-data'
            sched.ics
          end
        end
        responses << ["/calendar/#{sched.calendar.uri}/#{sched}", results]
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
        sched_uri = File.basename(uri)
        sched = Schedule.find_by_uri!(sched_uri)

        results = handle_props(props) do |prop|
          case prop
          when 'getetag'
            getetag(sched)
          when 'calendar-data'
            sched.ics
          end
        end
        responses << [uri.text, results]
      end
      responses
    end
  end

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
        when 'current-user-privilege-set'
          <<-EOS
            <A:privilege>
              <A:all />
              <A:read />
              <A:write />
              <A:write-properties />
              <A:write-content />
            </A:privilege>
EOS
        when 'supported-calendar-component-set'
          <<-EOS
            <CALDAV:comp name="VTODO" />
            <CALDAV:comp name="VEVENT" />
EOS
        when 'resourcetype'
          "<A:collection />"
        end
      end

      responses = [["/calendar/", results]]

      if params[:calendar] == ""
        if request.headers["Depth"] == "1"
          cals = Calendar.all
        else
          cals = []
        end
      else
        cals = [Calendar.find_by_uri!(params[:calendar])]
      end
     
      for cal in cals
        calprops = cal.props
        results = handle_props(props) do |prop|
          if calprops.key?(prop)
            calprops[prop]
          else
            case prop
            when 'supported-calendar-component-set'
          <<-EOS
            <CALDAV:comp name="VTODO" />
            <CALDAV:comp name="VEVENT" />
EOS
            when 'getctag'
              c = Change.where(calendar: cal).order('updated_at DESC').first
              (c)? Digest::MD5.hexdigest(c.id.to_s) : ''
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
      calendar = Calendar.find_by_uri!(params[:calendar])
      responses = []
      Schedule.where(calendar_id: calendar).find_each do |sched|
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

  def request_body
    request.body.seek(0)
    request.body.read.force_encoding("UTF-8")
  end                                              

  DEFAULT_NAMESPACES = {
    "DAV"    => "DAV:",
    "CALDAV" => "urn:ietf:params:xml:ns:caldav",
    "CS"     => "http://calendarserver.org/ns/",
    "ICAL"   => "http://apple.com/ns/ical/",
    "ME"     => "http://me.com/_namespace/"
  }

  def replace_propxml_nsprefix(xml, s)
    xml.collect_namespaces.each do |k, v|
      replace_to = DEFAULT_NAMESPACES.select {|_, w| v == w }.keys

      if replace_to != []
        from = k.sub('xmlns:', '')
        to = replace_to[0]
        s.gsub!("<#{from}:", "<#{to}:")
        s.gsub!("</#{from}:", "</#{to}:")
      end
    end

    s
  end

  def respond_xml_request(xpath)
    xml = Nokogiri::XML(request_body)
    props = xml.xpath(xpath, A: 'DAV:', C: 'urn:ietf:params:xml:ns:caldav')
    responses = yield props

    namespaces = DEFAULT_NAMESPACES
    for prop in props
      prefix = prop.namespace.prefix
      unless namespaces.has_key?(prefix)
        namespaces[prefix] = prop.namespace.href
      end
    end

    create_multistatus_xml(responses, namespaces)
  end

  def create_multistatus_xml(responses, namespaces)
    ns = ""
    namespaces.each do |prefix, uri|
      ns += " xmlns:#{prefix}=\"#{uri}\""
    end

    xml = <<EOS
<?xml version="1.0" encoding="UTF-8"?>
<multistatus xmlns="DAV:" #{ns}>
EOS
    for href, results in responses
      xml += "<response>"
      xml += "  <href>#{href}</href>"
      for status, props in results
        xml += "<propstat>"
        xml += "  <status>#{stringify_http_status_code(status)}</status>"
        xml += "  <prop>"
        for ns, name, child in props
          if child
            xml += "<#{ns}:#{name}>"
            xml += child
            xml += "</#{ns}:#{name}>"
          else
            xml += "<#{ns}:#{name} />"
          end
        end
        xml += "  </prop>"
        xml += "</propstat>"
      end
      xml += "</response>"
    end
    xml += "</multistatus>"

    xml
  end

  def stringify_http_status_code(status)
    code = Rack::Utils::SYMBOL_TO_STATUS_CODE[status]
    desc = Rack::Utils::HTTP_STATUS_CODES[code]

    "HTTP/1.1 #{code} #{desc}"
  end

  def authenticate
    authenticate_or_request_with_http_basic('realm') do |name, password|
      @user = User.find_by_name(name)
      @user and @user.validate_password(password)
    end
  end
end
