class SmmsController < ApplicationController
  skip_before_action :authenticate

  def http
    begin
      device = SMMSService.receive(request.body.read)
    rescue ActionController::BadRequest => e
      logger.warn "invalid smms payload: #{e}"
      head :bad_request
      return
    end

    render body: SMMSService.payload_for(device)
  end
 end
