class HealthController < ApplicationController
  skip_before_action :authenticate
  def health
    status = {
      status: 'ok'
    }

    render status: :ok, json: status
  end
end
