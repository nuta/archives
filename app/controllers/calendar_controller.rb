class CalendarController < ApplicationController
  before_action :authenticate, except: [:options]

  def options
    headers['Allow'] = 'OPTIONS, GET, PUT, DELETE, MKCALENDAR, ' +
                       'PROPFIND, PROPPATCH, REPORT'
    headers['DAV']   = '1, 2, calendar-access'
    head :ok
  end

  def get
    entry = Schedule.find_by_uri(params[:uri])
    unless entry
      head :status => :not_found
      return
    end

    render :text => entry.ics, :status => :ok
  end

  def put
    uri  = params[:uri]
    body = request.body.read.force_encoding("UTF-8")
    ics  = ICS::parse(body)

    # TODO
    comp = "VEVENT"
    date_start = "20150919T190000"
    date_end   = "20150920T053000"
    summary    = "Hello World"
    uid        = "1234567890"

    entry = Schedule.where(uri: uri).first_or_create
    entry.uri  = uri
    entry.ics  = body
    entry.component = comp
    entry.summary    = summary
    entry.date_start = date_start
    entry.date_end   = date_end
    entry.uid        = uid
    entry.save

    head :status => :created
  end

  def delete
    entry = Schedule.find_by_uri(params[:uri])
    unless entry
      head :status => :not_found
      return
    end

    entry.delete
    head :status => :no_content
  end

  def mkcalendar
    body = request.body.read.force_encoding("UTF-8")
    xml  = Nokogiri::XML(body)
    name = xml.xpath('/B:mkcalendar/A:set/A:prop/A:displayname',
                     A: 'DAV:', B: 'urn:ietf:params:xml:ns:caldav').children.to_s

    calendar = Calendar.new
    calendar.name    = name
    calendar.propxml = body
    calendar.user    = @user
    calendar.save

    head :status => :created
  end

  def report
    render :text => 'not implemented', :status => :not_implemented
  end

  def proppatch
      uri = params['uri']
      patch = Nokogiri::XML(request.body.read.force_encoding("UTF-8"))
      props = patch.xpath('/A:propertyupdate/A:set/A:prop/*', A: 'DAV:')

      props_ok = []
      namespaces = {}
      for prop in props
        props_ok << [prop.namespace.prefix, prop.name]
        namespaces["xmlns:#{prop.namespace.prefix}".to_sym] = prop.namespace.href
      end

      # create a response XML
      res = Nokogiri::XML::Builder.new do |xml|
        xml.multistatus("xmlns" => "DAV:", **namespaces) {
          xml.response {
            xml.href uri
            xml.propstat {
              xml.prop {
                props_ok.each do |prefix, name, v|
                  xml[prefix].send(name, v)
                end
              }
              xml.status 'HTTP/1.1 200 OK'
            }
          }
        }
      end
      render :xml => res.to_xml, :status => :multi_status
  end

  def propfind
    uri = params['uri']

    # XXX
    if uri == ''
      uri = '/'
    end

    case uri
    when '/'
      # PROPFIND to / (root)
      xml = Nokogiri::XML(request.body.read.force_encoding("UTF-8"))
      props = xml.xpath('/A:propfind/A:prop/*', A: 'DAV:')

      props_ok = []         # properties we know what it is
      props_not_found = []  # properties we don't know
      namespaces = {}       # namespaces used in a response XML

      # process each properties in the request
      for prop in props
        v = nil
        case prop.name
        when 'displayname';                v = ''
        when 'calendar-home-set';          v = '/calendar/hello-world' # XXX
        when 'principal-URL';              v = '/'
        when 'principal-collection-set';   v = '/'
        end

        namespaces["xmlns:#{prop.namespace.prefix}".to_sym] = prop.namespace.href

        if v
          props_ok << [prop.namespace.prefix, prop.name, v]
        else
          props_not_found << [prop.namespace.prefix, prop.name, '']
        end
      end

      # create a response XML
      res = Nokogiri::XML::Builder.new do |xml|
        xml.multistatus("xmlns" => "DAV:", **namespaces) {
          xml.response {
            xml.href uri
            xml.propstat {
              xml.prop {
                props_ok.each do |prefix, name, v|
                  case name
                  when 'calendar-home-set'
                    xml[prefix].send(name) {
                      xml.href v
                    }
                  else
                    xml[prefix].send(name, v)
                  end
                end
              }
              xml.status 'HTTP/1.1 200 OK'
            }

            xml.propstat {
              xml.prop {
                props_not_found.each do |prefix, name, v|
                  xml[prefix].send(name, v)
                end
              }
              xml.status 'HTTP/1.1 404 Not Found'
            }
          }
        }
      end
      render :xml => res.to_xml, :status => :multi_status
    else
      # get a list of calendars
      cal_name = params['uri']

      xml = Nokogiri::XML(request.body.read.force_encoding("UTF-8"))
      props = xml.xpath('/A:propfind/A:prop/*', A: 'DAV:')

      cs = []               # calendars
      namespaces = {}       # namespaces used in a response XML
      caldav_ns = ''

      # process each properties in the request
      for cal in Calendar.all
        props_ok = []         # properties we know what it is
        props_not_found = []  # properties we don't know

        for prop in props
          v = nil
          # TODO
          case prop.name
          when 'calendar-color';                   v = '#00ff00'
          when 'calendar-order';                   v = '10'
          when 'displayname';                      v = 'No Name'
          when 'supported-calendar-component-set'; v = ['VEVENT']
          when 'resourcetype';                     v = ''
          end

          if prop.namespace.href == 'urn:ietf:params:xml:ns:caldav'
            caldav_ns = prop.namespace.prefix
          end
  
          namespaces["xmlns:#{prop.namespace.prefix}".to_sym] = prop.namespace.href

          if v
            props_ok << [prop.namespace.prefix, prop.name, v]
          else
            props_not_found << [prop.namespace.prefix, prop.name, '']
          end
        end

        cs << ["/calendar/#{cal_name}/", props_ok, props_not_found]
      end

      # create a response XML
      res = Nokogiri::XML::Builder.new do |xml|
        xml.multistatus("xmlns" => "DAV:", **namespaces) {
          for href, props_ok, props_not_found in cs
            xml.response {
              xml.href href
              xml.propstat {
                xml.prop {
                  props_ok.each do |prefix, name, v|
                    case name
                    when 'supported-calendar-component-set'
                      xml[prefix].send(name) {
                        for comp in v
                            xml[prefix].comp(:name => comp)
                        end
                      }
                    when 'resourcetype'
                      xml[prefix].send(name) {
                        xml[caldav_ns].calendar
                        xml.collection
                      }
                    else
                      xml[prefix].send(name, v)
                    end
                  end
                }
                xml.status 'HTTP/1.1 200 OK'
              }
  
              xml.propstat {
                xml.prop {
                  props_not_found.each do |prefix, name, v|
                    xml[prefix].send(name, v)
                  end
                }
                xml.status 'HTTP/1.1 404 Not Found'
              }
            }
          end
        }
      end
      render :xml => res.to_xml, :status => :multi_status
    end
  end

  private

  def authenticate
    # TODO: check password
    authenticate_or_request_with_http_basic('realm') do |name, passwd|
      @user = User.find_by_name(name)
      unless @user
        logger.warn "user '#{name}' not found"
        head :status => :not_found
        return
      end

      true
    end
  end
end
