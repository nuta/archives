Rails.application.routes.draw do
  root 'welcome#index'

  methods = %w(options get put delete mkcalendar propfind proppatch report)

  methods.each do |method|
    match '/calendar:uri', uri: /.*/, :to => 'calendar#' + method, :via => method.to_s
  end

  # other requests: return 404 Bad Request
  match '*otherwise', to: proc { [400, {}, ['']] }, via: methods
end
