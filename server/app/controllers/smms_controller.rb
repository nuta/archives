class SmmsController < ApplicationController
  skip_before_action :authenticate

  def http
    auth, timestamp, hmac = (request.headers['Authorization'] || '').split(' ')
    if auth != 'SMMS'
      raise ActionController::BadRequest.new(), "invalid Authorization header"
    end

    device = SMMSService.receive(request.body.read, timestamp, hmac)

    payload = SMMSService.payload_for(device)
    timestamp, hmac = SMMSService.sign(device, payload)
    response.headers['authorization'] = "SMMS #{timestamp} #{hmac}"
    render body: payload
  end
 end
