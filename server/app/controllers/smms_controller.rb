class SmmsController < ApplicationController
  skip_before_action :authenticate

  def http
    auth, timestamp, hmac = (request.headers['Authorization'] || '').split(' ')
    if auth != 'SMMS'
      logger.warn "invalid authentication header"
      head :forbidden
      return
    end

    begin
      device = SMMSService.receive(request.body.read, hmac_enabled: true,
                                   timestamp: timestamp, hmac: hmac)
    rescue ActionController::BadRequest => e
      logger.warn "invalid smms payload: #{e}"
      head :bad_request
      return
    end

    payload = SMMSService.payload_for(device)
    timestamp, hmac = SMMSService.sign(device, payload)
    response.headers['authorization'] = "SMMS #{timestamp} #{hmac}"

    sign_and_set_authorization_header(device, payload)
    render body: payload
  end
 end
