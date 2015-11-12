Rails.application.routes.draw do
  root 'welcome#index'

  methods = %w(options get put delete mkcalendar propfind proppatch report)

  methods.each do |method|
    match '/calendar:uri', uri: /.*/, :to => 'calendar#' + method, :via => method.to_s
  end
end
