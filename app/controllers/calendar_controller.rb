class CalendarController < ApplicationController
  before_action :authenticate, except: [:options]

  def options
    headers['Allow'] = 'OPTIONS, GET, PUT, DELETE, MKCALENDAR, '
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
    # TODO: Support overwriting. Note that HTTP status code should be
    #       204 on success.

    entry = Schedule.new
    entry.uri = params[:uri]
    entry.ics = request.body.read
    # TODO
    # entry.type =
    # entry.summary =
    # entry.date_start =
    # entry.date_end =
    # entry.uid =
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
    body = request.body.read
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
    render :text => 'not implemented', :status => :not_implemented
  end

  def propfind
    if params['uri'] == '' or params['uri'] == ''
      # PROPFIND to / (root)
      xml = Nokogiri::XML(request.body.read)
      props = xml.xpath('/A:propfind/A:prop/*', A: 'DAV:')

      props_ok = []         # properties we know what it is
      props_not_found = []  # properties we don't know
      namespaces = {}       # namespaces used in a response XML

      # process each properties in the request
      for prop in props
        namespaces["xmlns:#{prop.namespace.prefix}".to_sym] = prop.namespace.href

        # FIXME: ugly
        case prop.namespace.href
        when 'urn:ietf:params:xml:ns:caldav'
           case prop.name
           when 'calendar-home-set'
             props_ok << [prop, '/']
             next
           end
        when 'DAV:'
           case prop.name
           when 'displayname'
             props_ok << [prop, '']
             next
           end
        end

          props_not_found << [prop, '']
      end

      # create a response XML
      res = Nokogiri::XML::Builder.new do |xml|
        xml.multistatus(**namespaces) {
          xml.response {
            xml.propstat {
              xml.prop {
                props_ok.each do |prop, v|
                  xml[prop.namespace.prefix].send(prop.name, v)
                end
              }
              xml.status 'HTTP/1.1 200 OK'
            }

            xml.propstat {
              xml.prop {
                props_not_found.each do |prop, v|
                  xml[prop.namespace.prefix].send(prop.name, v)
                end
              }
              xml.status 'HTTP/1.1 404 Not Found'
            }
          }
        }
      end

      render :xml => res.to_xml, :status => :multi_status
    else
      render :text => 'not implemented', :status => :not_implemented
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
