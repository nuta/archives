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

    sched = Schedule.where(uri: params[:calendar_object]).first

    if request.headers.key?("If-None-Match") and sched
      head :status => :precondition_failed
      return
    end

    if request.headers.key?("If-Match") and
      getetag(sched) != request.headers["If-Match"]
      head :status => :precondition_failed
      return
    end

    entry = Schedule.where(uri: params[:calendar_object]).first_or_create
    entry.ics        = body
    entry.component  = comp_name
    entry.date_start = date_start
    entry.date_end   = date_end
    entry.calendar   = calendar
    entry.uid        = comp['uid']
    entry.summary    = comp['summary']
    entry.save

    head :status => :created
  end

  def delete
    entry = Schedule.find_by_uri!(params[:calendar_object])
    entry.delete
    head :status => :no_content
  end

  def mkcalendar
    body = request_body
    xml  = Nokogiri::XML(body)
    name = xml.xpath('/B:mkcalendar/A:set/A:prop/A:displayname',
                     A: 'DAV:', B: 'urn:ietf:params:xml:ns:caldav').children.to_s
    calendar = Calendar.new
    calendar.uri     = params[:calendar]
    calendar.name    = name
    calendar.propxml = body
    calendar.user    = @user
    calendar.save

    head :status => :created
  end

  def report
    xml = Nokogiri::XML(request_body)
    res = case xml.children[0].name
          when 'calendar-multiget'
            report_multiget(xml)
          else
            head :status => :not_implemented
            return
          end

    render :xml => res, :status => :multi_status
  end

  def proppatch
    # TODO
    xml = respond_xml_request('/A:propertyupdate/A:set/A:prop/*') do |props|
      results = handle_props(props) {|prop| '' }
      [["/calendar/#{params[:calendar]}/", results]]
    end

    render :xml => xml, :status => :multi_status
  end

  def propfind
    if params[:calendar] == 'calendars'
      xml = propfind_list_calendars
    elsif params[:calendar] != ''
      xml = propfind_list_objects
    else
      xml = propfind_root
    end

    render :xml => xml, :status => :multi_status
  end

  private

  def getetag(sched)
    Digest::MD5.hexdigest(sched.ics)
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

  def propfind_root
    # PROPFIND to / (root)
    respond_xml_request('/A:propfind/A:prop/*') do |props|
      results = handle_props(props) do |prop|
        case prop
        when 'displayname'
          ''
        when 'calendar-home-set'
          '<A:href>/calendar/calendars</A:href>'
        end
      end

      [["/", results]]
    end
  end

  def propfind_list_calendars
    # get a list of calendars
    respond_xml_request('/A:propfind/A:prop/*') do |props|
      results = handle_props(props) do |prop|
        case prop
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
      responses = [["/calendar/calendars/", results]]

      for cal in Calendar.all
        propxml = Nokogiri::XML(cal.propxml)
        results = handle_props(props) do |prop|
          case prop
          when 'calendar-color'
            '#ff00ff'
          when 'calendar-order'
            '10'
          when 'displayname'
            cal.name
          when 'supported-calendar-component-set'
              '<CALDAV:comp name="VEVENT" />'
          when 'resourcetype'
            <<-EOS
              <CALDAV:calendar />
              <A:collection />
EOS
          end
        end
        responses << ["/calendar/#{cal.uri}/", results]
      end
      responses
    end
  end

  def propfind_list_objects
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

  def respond_xml_request(xpath)
    xml = Nokogiri::XML(request_body)
    props = xml.xpath(xpath, A: 'DAV:', C: 'urn:ietf:params:xml:ns:caldav')
    responses = yield props

    namespaces = {
      'xmlns:CALDAV' => 'urn:ietf:params:xml:ns:caldav'
    }

    for prop in props
      prefix = "xmlns:#{prop.namespace.prefix}".to_sym
      unless namespaces.has_key?(prefix)
        namespaces[prefix] = prop.namespace.href
      end
    end

    create_multistatus_xml(responses, namespaces)
  end

  def create_multistatus_xml(responses, namespaces)
    ns = ""
    namespaces.each do |prefix, uri|
      ns += " #{prefix}=\"#{uri}\""
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

      if not @user or @user.password != password
        logger.warn "user '#{name}' not found"
        head :status => :forbidden
        return
      end

      true
    end
  end
end
