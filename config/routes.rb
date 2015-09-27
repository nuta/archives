Rails.application.routes.draw do
  root 'welcome#index'

  ['options', 'get', 'put', 'delete', 'mkcalendar',
   'propfind', 'proppatch', 'report'].each do |method|
    match '/:user/calendar/:uri', uri: /.*/, :to => 'calendar#' + method, :via => method.to_s
  end
end
