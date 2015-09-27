class CalendarController < ApplicationController
  def options
    headers['Allow'] = 'OPTIONS, GET, PUT, DELETE, MKCALENDAR, PROPFIND, PROPPATCH, REPORT'
    headers['DAV']   = ' 1, 2, calendar-access'
    head :ok
  end

  def get
    render :text => 'not implemented', :status => :not_implemented
  end

  def put
    render :text => 'not implemented', :status => :not_implemented
  end

  def delete
    render :text => 'not implemented', :status => :not_implemented
  end

  def mkcalendar
    render :text => 'not implemented', :status => :not_implemented
  end

  def report
    render :text => 'not implemented', :status => :not_implemented
  end

  def proppatch
    render :text => 'not implemented', :status => :not_implemented
  end

  def propfind
    res = ""
    render :text => res,
           :content_type => 'application/xml',
           :status => :multi_status
  end
end
