module DavXml
  extend ActiveSupport::Concern

  DEFAULT_NAMESPACES = {
    "DAV"    => "DAV:",
    "CALDAV" => "urn:ietf:params:xml:ns:caldav",
    "CS"     => "http://calendarserver.org/ns/",
    "ICAL"   => "http://apple.com/ns/ical/",
    "ME"     => "http://me.com/_namespace/"
  }

  def replace_xml_nsprefix(xml, s)
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
    xml = Nokogiri::XML(rawrequest)
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
            xml += "<#{ns}:#{name}>#{child}</#{ns}:#{name}>"
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
end
