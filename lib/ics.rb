#
#  iCalendar (RFC5545) Parser
#
module ICS
  extend self

  class ICalendar
    def initialize(ics)
      @ics = ICS::parse(ics)
      @default_comp_type = self.comp_type
    end

    # Returns the content of property `k`
    def comp(k, date=false)
      if date
        ICS::parse_date(@ics[@default_comp_type][0], k)
      else
        @ics[@default_comp_type][0][k]
      end
    end

    # Returns the first component type
    def comp_type
      # XXX
      for comp_type in %w(VEVENT VTODO)
        if @ics.has_key?(comp_type)
          return comp_type
        end
      end

      logger.warn 'unknown iCalendar component, handling as a VEVENT'
      return 'VEVENT'
    end
  end

  def parse_date(ics, prop)
    k = nil
    for key in ics.keys
      if key.start_with?(prop)
        k = key
      end
    end

    unless k
      return nil # property not found
    end

    if k.include?(';TZID=')
      timezone = k.split(';TZID=')[1]
    else
      timezone = 'UTC' # TZID is not speicifed
    end

    parser = ActiveSupport::TimeZone[timezone]
    if parser
      parser.parse(ics[k]).utc
    else
      return nil # invalid TZID
    end
  end

  def parse(text)
    name, block = parse_block(concat_lines(text))
    if name != 'VCALENDAR'
      raise 'text does not start with BEGIN:VCALENDAR'
    end

    block
  end

  private

  # parse between BEGIN:FOO and END:FOO
  def parse_block(lines)
    if lines.size == 0
      return '', {}, 0
    end

    comp_name = get_comp_name(lines[0])
    block = {}
    i = 1
    while i < lines.size
      l = lines[i]
      case l
      when "END:#{comp_name}"
        return comp_name, block, i
      when /^BEGIN:/
        name, sub_block, inc = parse_block(lines[i..-1])
        block[name] ||= []
        block[name] << sub_block
        i += inc
      else
        prop_name,val = l.split(':', 2)
        block[prop_name] = val
      end

      i += 1
    end

    raise "missing END:#{comp_name}"
  end

  def get_comp_name(begin_line)
    begin_line.sub('BEGIN:', '')
  end

  # Concatenates divided lines and remove CR and LF
  def concat_lines(text)

    #  Lines are separated by CR+LF and long one is
    #  divided into multiple lines by inserting CR+LF+WS
    #  (WS is *single* horizontal tab or white spate)
    lines = []
    for l in text.split(/\r?\n/)
      if l.strip == ''
        next
      end

      if l =~ /^\s|\t/
        # continued line
        line += l[1..-1]
      else
        if line
          lines << line
        end
        line = l
      end
    end

    if line
      lines << line
    end

    lines
  end
end
