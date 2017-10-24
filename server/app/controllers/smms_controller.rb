class SmmsController < ApplicationController
  skip_before_action :authenticate

  def http
    auth, timestamp, hmac = (request.headers['Authorization'] || '').split(' ')
    if auth != 'SMMS'
      head :forbidden
      return
    end

    begin
      device = SMMSService.receive(request.body.read, timestamp, hmac)
    rescue ActionController::BadRequest
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
