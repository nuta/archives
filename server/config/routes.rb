Rails.application.routes.draw do
  scope :api do
    if Rails.env.development?
      require 'sidekiq/web'      
      mount Sidekiq::Web => '/sidekiq'
    end
    
    scope :v1 do
      mount_devise_token_auth_for 'User', at: 'auth'
  
      resources :apps, param: :app_name, format: false
      scope 'apps/:app_name' do
        resources :deployments, param: :version, format: false, only: %i(index show create)
        resources :app_stores, path: 'stores', param: :key, format: false, constraints: { key: /.+/ }
        resources :integrations, param: :service, format: false
        
        get "log", to: "apps#log"
        get "files", to: "source_files#index"
        get "files/*path", to: "source_files#show", constraints: { path: /.+/ }
        put "files/*path", to: "source_files#update", constraints: { path: /.+/ }
        delete "files/*path", to: "source_files#destroy", constraints: { path: /.+/ }
      end

      resources :devices, param: :device_name, format: false
      scope 'devices/:device_name' do
        resources :device_stores, path: 'stores', param: :key, format: false, constraints: { key: /.+/ }
        get "log", to: "devices#log"
      end

      post "heartbeat", to: "heartbeat#heartbeat"
      get "os_image",   to: "heartbeat#os_image"
      get "app_image",  to: "heartbeat#app_image"
  end
  end
end