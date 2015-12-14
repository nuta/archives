Rails.application.routes.draw do
  root to: proc { [200, {}, ['']] }

  methods = %w(options get put delete mkcalendar propfind proppatch report)

  methods.each do |method|
    with_options to: 'calendar#' + method, via: method.to_s do
      match '/calendar', calendar: '', calendar_object: ''
      match '/calendar/:calendar', calendar_object: ''
      match '/calendar/:calendar/*calendar_object'
    end
  end

  # other requests: return 404 Bad Request
  methods.each do |method|
    match '/*otherwise', to: proc { [404, {}, ['']] }, via: method

    # return 200 to prevent annoying OS X calendar error prompts
    match '/', to: proc { [200, {}, ['']] }, via: method
  end
end
