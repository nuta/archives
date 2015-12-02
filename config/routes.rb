Rails.application.routes.draw do
  root 'welcome#index'

  methods = %w(options get put delete mkcalendar propfind proppatch report)

  methods.each do |method|
    match '/calendar', calendar: '', calendar_object: '',
          to: 'calendar#' + method, via: method.to_s
    match '/calendar/:calendar', calendar_object: '',
          to: 'calendar#' + method, via: method.to_s
    match '/calendar/:calendar/*calendar_object',
          to: 'calendar#' + method, via: method.to_s
  end

  # other requests: return 404 Bad Request
  methods.each do |method|
    match '*otherwise', to: proc { [400, {}, ['']] }, via: methods
  end
end
