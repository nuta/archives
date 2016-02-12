class WellKnownController < ApplicationController
  def redirect
    case params[:type]
    when 'caldav'
      redirect_to '/calendar/', status: :moved_permanently
    else
      head :not_implemented
    end
  end
end
