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
    body = request.body.read.force_encoding("UTF-8")
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
    body = request.body.read.force_encoding("UTF-8")
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
    xml = Nokogiri::XML(request.body.read.force_encoding("UTF-8"))
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
      [[params[:calendar], results]]
    end

    render :xml => xml, :status => :multi_status
  end

  def propfind
    if params[:calendar] == 'calendars'
      xml = propfind_list_calendars
    elsif params[:calendar_object] != ''
      xml = propfind_list_objects
    else
      xml = propfind_root
    end

    render :xml => xml, :status => :multi_status
  end

  private

  def report_multiget(xml)
    respond_xml_request('/C:calendar-multiget/A:prop/*') do |props|
      uris  = xml.xpath('/C:calendar-multiget/A:href',
                        A: 'DAV:', C: 'urn:ietf:params:xml:ns:caldav')
      responses = []      
      for uri in uris
        results = handle_props(props) do |prop|
          sched = Schedule.find_by_uri!(uri.text)
          case prop
          when 'getetag'
            Digest::MD5.hexdigest(sched.ics)
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
          {prefix: 'A', name: 'href', value: '/calendar/calendars'}
        end
      end

      [["#{params[:calendar]}/#{params[:calendar_object]}", results]]
    end
  end

  def propfind_list_calendars
    # get a list of calendars
    respond_xml_request('/A:propfind/A:prop/*') do |props|
      responses = []
      for cal in Calendar.all
        results = handle_props(props) do |prop|
          case prop
          when 'calendar-color'                   
              '#00ff00'
          when 'calendar-order'                   
              '10'
          when 'displayname'                      
              'No Name'
          when 'supported-calendar-component-set' 
              {prefix: 'C', name: 'comp', attrs: {:name => 'VEVENT'}}
          when 'resourcetype'                     
              ''
          end
        end
        responses << ["/calendar/#{cal.uri}", results]
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
            Digest::MD5.hexdigest(sched.ics)
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

  def respond_xml_request(xpath)
    xml = Nokogiri::XML(request.body.read.force_encoding("UTF-8"))
    props = xml.xpath(xpath, A: 'DAV:', C: 'urn:ietf:params:xml:ns:caldav')
    responses = yield props

    namespaces = {}
    for prop in props
      prefix = "xmlns:#{prop.namespace.prefix}".to_sym
      unless namespaces.has_key?(prefix)
        namespaces[prefix] = prop.namespace.href
      end
    end

    create_multistatus_xml(responses, namespaces)
  end

  def create_multistatus_xml(responses, namespaces)
    r = Nokogiri::XML::Builder.new do |xml|
      xml.multistatus("xmlns" => "DAV:", **namespaces) do
        for href, results in responses
          xml.response do
            xml.href href
            for status, props in results
              xml.propstat do
                xml.status stringify_http_status_code(status)
                xml.prop do  
                  for prefix, name, child in props
                    if child.class == Hash
                      xml[prefix].send(name) do
                        attrs = (child[:attrs])? child[:attrs] : {}
                        xml[child[:prefix]].send(child[:name], child[:value],
                                                 attrs)
                      end
                    else
                      xml[prefix].send(name, child)
                    end
                  end
                end
              end
            end
          end
        end
      end
    end.to_xml
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
