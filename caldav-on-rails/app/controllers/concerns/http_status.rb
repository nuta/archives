module HttpStatus
  extend ActiveSupport::Concern

  def stringify_http_status_code(status)
    code = Rack::Utils::SYMBOL_TO_STATUS_CODE[status]
    desc = Rack::Utils::HTTP_STATUS_CODES[code]

    "HTTP/1.1 #{code} #{desc}"
  end
end
