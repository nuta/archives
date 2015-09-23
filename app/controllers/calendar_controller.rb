class CalendarController < ApplicationController
  def options
    headers['Allow'] = 'OPTIONS, GET, PUT, DELETE, MKCALENDAR, PROPFIND'
    headers['DAV']   = ' 1, 2, calendar-access'
    head :ok
  end

  def get
    render :text => 'not implemented'
  end

  def put
    render :text => 'not implemented'
  end

  def delete
    render :text => 'not implemented'
  end

  def mkcalendar
    render :text => 'not implemented'
  end

  def propfind
    render :text => 'not implemented'
  end
end
