class SmmsController < ApplicationController
  skip_before_action :authenticate

  def http
    device = SMMSService.receive(request.body.read)
    render body: SMMSService.payload_for(device)
  end
 end
